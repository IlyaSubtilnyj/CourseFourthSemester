#include "Net.h"

namespace net
{

#if PLATFORM == PLATFORM_WINDOWS

	void wait(float seconds)
	{
		Sleep((int)(seconds * 1000.0f));
	}

#else

#include <unistd.h>
	void wait(float seconds) { usleep((int)(seconds * 1000000.0f)); }

#endif

	//address class
	Address::Address()
		:
		address(0),
		port(0)
	{}

	Address::Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port)
		:
		address((a << 24) | (b << 16) | (c << 8) | d),
		port(port)
	{}

	Address::Address(unsigned int address, unsigned short port)
		:
		address(address),
		port(port)
	{}

	
	unsigned int Address::GetAddress() const
	{
		return address;
	}

	unsigned char Address::GetA() const
	{
		return (unsigned char)(address >> 24);
	}

	unsigned char Address::GetB() const
	{
		return (unsigned char)(address >> 16);
	}

	unsigned char Address::GetC() const
	{
		return (unsigned char)(address >> 8);
	}

	unsigned char Address::GetD() const
	{
		return (unsigned char)(address);
	}

	unsigned short Address::GetPort() const
	{
		return port;
	}

	bool Address::operator == (const Address& other) const
	{
		return address == other.address && port == other.port;
	}

	bool Address::operator != (const Address& other) const
	{
		return !(*this == other);
	}

	bool Address::operator < (const Address& other) const
	{
		// note: this is so we can use address as a key in std::map
		if (address < other.address)
			return true;
		if (address > other.address)
			return false;
		else
			return port < other.port;
	}

	//socket
	Socket::Socket()
	{
		socket = 0;
	}

	Socket::~Socket()
	{
		Close();
	}
	
	char* Socket::get_local_ip()
	{
		char host[256];
		char* IP;
		struct hostent* host_entry;
		int hostname;
		hostname = gethostname(host, sizeof(host));
		host_entry = gethostbyname(host);
		IP = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
		return IP;
	}

	bool Socket::Open(const char* port)
	{
		struct addrinfo hints, * servinfo, * p;
		int rv;

		char* client_ip = get_local_ip();

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		if (rv = getaddrinfo(client_ip, port, &hints, &servinfo) != 0)
			return false;

		for (p = servinfo; p != nullptr; p = p->ai_next)
		{
			if ((socket = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == SOCKET_ERROR)
			{
				continue;
			}

			if (bind(socket, p->ai_addr, p->ai_addrlen) == 0)
			{
				break;
			}

			closesocket(socket);
		}

		if (p != nullptr)
		{
			struct sockaddr_in* s = (struct sockaddr_in*)p->ai_addr;
			u_short p_port = htons(s->sin_port);
			printf("port: %d\n", p_port);

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

			int nonBlocking = 1;
			if (fcntl(socket, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
			{
				printf("failed to set non-blocking socket\n");
				Close();
				return false;
		}

#elif PLATFORM == PLATFORM_WINDOWS

			DWORD nonBlocking = 1;
			if (ioctlsocket(socket, FIONBIO, &nonBlocking) != 0)
			{
				printf("failed to set non-blocking socket\n");
				Close();
				return false;
			}
#endif
		}
		else return false;

		return true;
	}

	void Socket::Close()
	{
		if (socket != 0)
		{
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
			close(socket);
#elif PLATFORM == PLATFORM_WINDOWS
			closesocket(socket);
#endif
			socket = 0;
		}
	}

	bool Socket::IsOpen() const
	{
		return socket != 0;
	}

	bool Socket::Send(const Address& destination, const void* data, int size)
	{
		assert(data);
		assert(size > 0);

		if (socket == 0)
			return false;

		assert(destination.GetAddress() != 0);
		assert(destination.GetPort() != 0);

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(destination.GetAddress());
		address.sin_port = htons((unsigned short)destination.GetPort());

		int sent_bytes = sendto(socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));

		return sent_bytes == size;
	}

	int Socket::Receive(Address& sender, void* data, int size)
	{
		assert(data);
		assert(size > 0);

		if (socket == 0)
			return false;

#if PLATFORM == PLATFORM_WINDOWS
		typedef int socklen_t;
#endif

		sockaddr_in from;
		socklen_t fromLength = sizeof(from);

		int received_bytes = recvfrom(socket, (char*)data, size, 0, (sockaddr*)&from, &fromLength);

		if (received_bytes <= 0)
			return 0;

		unsigned int address = ntohl(from.sin_addr.s_addr);
		unsigned short port = ntohs(from.sin_port);

		sender = Address(address, port);

		return received_bytes;
	}

	// packet queue
	bool PacketQueue::exists(unsigned int sequence)
	{
		for (iterator itor = begin(); itor != end(); ++itor)
			if (itor->sequence == sequence)
				return true;
		return false;
	}

	void PacketQueue::insert_sorted(const PacketData& p, unsigned int max_sequence)
	{
		if (empty())
		{
			push_back(p);
		}
		else
		{
			if (!sequence_more_recent(p.sequence, front().sequence, max_sequence))
			{
				push_front(p);
			}
			else if (sequence_more_recent(p.sequence, back().sequence, max_sequence))
			{
				push_back(p);
			}
			else
			{
				for (PacketQueue::iterator itor = begin(); itor != end(); itor++)
				{
					assert(itor->sequence != p.sequence);
					if (sequence_more_recent(itor->sequence, p.sequence, max_sequence))
					{
						insert(itor, p);
						break;
					}
				}
			}
		}
	}

	void PacketQueue::verify_sorted(unsigned int max_sequence)
	{
		PacketQueue::iterator prev = end();
		for (PacketQueue::iterator itor = begin(); itor != end(); itor++)
		{
			assert(itor->sequence <= max_sequence);
			if (prev != end())
			{
				assert(sequence_more_recent(itor->sequence, prev->sequence, max_sequence));
				prev = itor;
			}
		}
	}


	//reliability system
	void ReliabilitySystem::SetMaxSeq(unsigned int max_sequence)
	{
		ReliabilitySystem::max_sequence = max_sequence;
	}

	ReliabilitySystem::ReliabilitySystem(unsigned int max_sequence)
	{
		this->rtt_maximum = rtt_maximum;
		this->max_sequence = max_sequence;
		Reset();
	}

	void ReliabilitySystem::Reset()
	{
		local_sequence = 0;
		remote_sequence = 0;
		sentQueue.clear();
		receivedQueue.clear();
		pendingAckQueue.clear();
		ackedQueue.clear();
		sent_packets = 0;
		recv_packets = 0;
		lost_packets = 0;
		acked_packets = 0;
		sent_bandwidth = 0.0f;
		acked_bandwidth = 0.0f;
		rtt = 0.0f;
		rtt_maximum = 1.0f;
	}

	void ReliabilitySystem::PacketSent(int size)
	{
		if (sentQueue.exists(local_sequence))
		{
			printf("local sequence %d exists\n", local_sequence);
			for (PacketQueue::iterator itor = sentQueue.begin(); itor != sentQueue.end(); ++itor)
				printf(" + %d\n", itor->sequence);
		}
		assert(!sentQueue.exists(local_sequence));
		assert(!pendingAckQueue.exists(local_sequence));
		PacketData data;
		data.sequence = local_sequence;
		data.time = 0.0f;
		data.size = size;
		sentQueue.push_back(data);
		pendingAckQueue.push_back(data);
		sent_packets++;
		local_sequence++;
		if (local_sequence > max_sequence)
			local_sequence = 0;
	}

	void ReliabilitySystem::PacketReceived(unsigned int sequence, int size)
	{
		recv_packets++;
		if (receivedQueue.exists(sequence))
			return;
		PacketData data;
		data.sequence = sequence;
		data.time = 0.0f;
		data.size = size;
		receivedQueue.push_back(data);
		if (sequence_more_recent(sequence, remote_sequence, max_sequence))
			remote_sequence = sequence;
	}

	unsigned int ReliabilitySystem::GenerateAckBits()
	{
		return generate_ack_bits(GetRemoteSequence(), receivedQueue, max_sequence);
	}

	void ReliabilitySystem::ProcessAck(unsigned int ack, unsigned int ack_bits)
	{
		process_ack(ack, ack_bits, pendingAckQueue, ackedQueue, acks, acked_packets, rtt, max_sequence);
	}

	void ReliabilitySystem::Update(float deltaTime)
	{
		acks.clear();
		AdvanceQueueTime(deltaTime);
		UpdateQueues();
		UpdateStats();
	}

	void ReliabilitySystem::Validate()
	{
		sentQueue.verify_sorted(max_sequence);
		receivedQueue.verify_sorted(max_sequence);
		pendingAckQueue.verify_sorted(max_sequence);
		ackedQueue.verify_sorted(max_sequence);
	}

	// utility functions

	bool ReliabilitySystem::sequence_more_recent(unsigned int s1, unsigned int s2, unsigned int max_sequence)
	{
		return (s1 > s2) && (s1 - s2 <= max_sequence / 2) || (s2 > s1) && (s2 - s1 > max_sequence / 2);
	}

	int ReliabilitySystem::bit_index_for_sequence(unsigned int sequence, unsigned int ack, unsigned int max_sequence)
	{
		assert(sequence != ack);
		assert(!sequence_more_recent(sequence, ack, max_sequence));
		if (sequence > ack)
		{
			assert(ack < 33);
			assert(max_sequence >= sequence);
			return ack + (max_sequence - sequence);
		}
		else
		{
			assert(ack >= 1);
			assert(sequence <= ack - 1);
			return ack - 1 - sequence;
		}
	}

	unsigned int ReliabilitySystem::generate_ack_bits(unsigned int ack, const PacketQueue& received_queue, unsigned int max_sequence)
	{
		unsigned int ack_bits = 0;
		for (PacketQueue::const_iterator itor = received_queue.begin(); itor != received_queue.end(); itor++)
		{
			if (itor->sequence == ack || sequence_more_recent(itor->sequence, ack, max_sequence))
				break;
			int bit_index = bit_index_for_sequence(itor->sequence, ack, max_sequence);
			if (bit_index <= 31)
				ack_bits |= 1 << bit_index;
		}
		return ack_bits;
	}

	void ReliabilitySystem::process_ack(unsigned int ack, unsigned int ack_bits,
		PacketQueue& pending_ack_queue, PacketQueue& acked_queue,
		std::vector<unsigned int>& acks, unsigned int& acked_packets,
		float& rtt, unsigned int max_sequence)
	{
		if (pending_ack_queue.empty())
			return;

		PacketQueue::iterator itor = pending_ack_queue.begin();
		while (itor != pending_ack_queue.end())
		{
			bool acked = false;

			if (itor->sequence == ack)
			{
				acked = true;
			}
			else if (!sequence_more_recent(itor->sequence, ack, max_sequence))
			{
				int bit_index = bit_index_for_sequence(itor->sequence, ack, max_sequence);
				if (bit_index <= 31)
					acked = (ack_bits >> bit_index) & 1;
			}

			if (acked)
			{
				rtt += (itor->time - rtt) * 0.1f;

				acked_queue.insert_sorted(*itor, max_sequence);
				acks.push_back(itor->sequence);
				acked_packets++;
				itor = pending_ack_queue.erase(itor);
			}
			else
				++itor;
		}
	}

	// data accessors

	unsigned int ReliabilitySystem::GetLocalSequence() const
	{
		return local_sequence;
	}

	unsigned int ReliabilitySystem::GetRemoteSequence() const
	{
		return remote_sequence;
	}

	unsigned int ReliabilitySystem::GetMaxSequence() const
	{
		return max_sequence;
	}

	void ReliabilitySystem::GetAcks(unsigned int** acks, int& count)
	{
		*acks = &this->acks[0];
		count = (int)this->acks.size();
	}

	unsigned int ReliabilitySystem::GetSentPackets() const
	{
		return sent_packets;
	}

	unsigned int ReliabilitySystem::GetReceivedPackets() const
	{
		return recv_packets;
	}

	unsigned int ReliabilitySystem::GetLostPackets() const
	{
		return lost_packets;
	}

	unsigned int ReliabilitySystem::GetAckedPackets() const
	{
		return acked_packets;
	}

	float ReliabilitySystem::GetSentBandwidth() const
	{
		return sent_bandwidth;
	}

	float ReliabilitySystem::GetAckedBandwidth() const
	{
		return acked_bandwidth;
	}

	float ReliabilitySystem::GetRoundTripTime() const
	{
		return rtt;
	}

	int ReliabilitySystem::GetHeaderSize() const
	{
		return 12;
	}

	void ReliabilitySystem::AdvanceQueueTime(float deltaTime)
	{
		for (PacketQueue::iterator itor = sentQueue.begin(); itor != sentQueue.end(); itor++)
			itor->time += deltaTime;

		for (PacketQueue::iterator itor = receivedQueue.begin(); itor != receivedQueue.end(); itor++)
			itor->time += deltaTime;

		for (PacketQueue::iterator itor = pendingAckQueue.begin(); itor != pendingAckQueue.end(); itor++)
			itor->time += deltaTime;

		for (PacketQueue::iterator itor = ackedQueue.begin(); itor != ackedQueue.end(); itor++)
			itor->time += deltaTime;
	}

	void ReliabilitySystem::UpdateQueues()
	{
		const float epsilon = 0.001f;

		while (sentQueue.size() && sentQueue.front().time > rtt_maximum + epsilon)
			sentQueue.pop_front();

		if (receivedQueue.size())
		{
			const unsigned int latest_sequence = receivedQueue.back().sequence;
			const unsigned int minimum_sequence = latest_sequence >= 34 ? (latest_sequence - 34) : max_sequence - (34 - latest_sequence);
			while (receivedQueue.size() && !sequence_more_recent(receivedQueue.front().sequence, minimum_sequence, max_sequence))
				receivedQueue.pop_front();
		}

		while (ackedQueue.size() && ackedQueue.front().time > rtt_maximum * 2 - epsilon)
			ackedQueue.pop_front();

		while (pendingAckQueue.size() && pendingAckQueue.front().time > rtt_maximum + epsilon)
		{
			pendingAckQueue.pop_front();
			lost_packets++;
		}
	}

	void ReliabilitySystem::UpdateStats()
	{
		int sent_bytes_per_second = 0;
		for (PacketQueue::iterator itor = sentQueue.begin(); itor != sentQueue.end(); ++itor)
			sent_bytes_per_second += itor->size;
		int acked_packets_per_second = 0;
		int acked_bytes_per_second = 0;
		for (PacketQueue::iterator itor = ackedQueue.begin(); itor != ackedQueue.end(); ++itor)
		{
			if (itor->time >= rtt_maximum)
			{
				acked_packets_per_second++;
				acked_bytes_per_second += itor->size;
			}
		}
		sent_bytes_per_second /= rtt_maximum;
		acked_bytes_per_second /= rtt_maximum;
		sent_bandwidth = sent_bytes_per_second * (8 / 1000.0f);
		acked_bandwidth = acked_bytes_per_second * (8 / 1000.0f);
	}

	unsigned int ReliabilitySystem::max_sequence = 0xFFFFFFFF;

	//connection
	Connection::Connection(unsigned int protocolId, float timeout)
	{
		this->protocolId = protocolId;
		this->timeout = timeout;
		mode = None;
		running = false;
		ClearData();
	}

	Connection::~Connection()
	{
		if (IsRunning())
			Stop();
	}

	bool Connection::Start(const char* port)
	{
		assert(!running);
		if (!socket.Open(port))
			return false;
		running = true;
		OnStart();
		return true;
	}

	void Connection::Stop()
	{
		assert(running);
		printf("stop connection\n");
		bool connected = IsConnected();
		ClearData();
		socket.Close();
		running = false;
		if (connected)
			OnDisconnect();
		OnStop();
	}

	bool Connection::IsRunning() const
	{
		return running;
	}

	void Connection::Listen()
	{
		printf("server listening for connection\n");
		bool connected = IsConnected();
		ClearData();
		if (connected)
			OnDisconnect();
		mode = Server;
		state = Listening;
	}

	void Connection::Connect(const Address& address)
	{
		printf("client connecting to %d.%d.%d.%d:%d\n",
			address.GetA(), address.GetB(), address.GetC(), address.GetD(), address.GetPort());
		bool connected = IsConnected();
		ClearData();
		if (connected)
			OnDisconnect();
		mode = Client;
		state = Connecting;
		this->address = address;
	}

	bool Connection::IsDisconnected() const
	{
		return state == Disconnected;
	}

	bool Connection::IsConnecting() const
	{
		return state == Connecting;
	}

	bool Connection::ConnectFailed() const
	{
		return state == ConnectFail;
	}

	bool Connection::IsConnected() const
	{
		return state == Connected;
	}

	bool Connection::IsListening() const
	{
		return state == Listening;
	}

	Connection::Mode Connection::GetMode() const
	{
		return mode;
	}

	void Connection::Update(float deltaTime)
	{
		assert(running);
		timeoutAccumulator += deltaTime;
		if (timeoutAccumulator > timeout)
		{
			if (state == Connecting)
			{
				printf("connect timed out\n");
				ClearData();
				state = ConnectFail;
				OnDisconnect();
			}
			else if (state == Connected)
			{
				printf("connection timed out\n");
				ClearData();
				if (state == Connecting)
					state = ConnectFail;
				OnDisconnect();
			}
		}
	}

	bool Connection::SendPacket(const unsigned char data[], int size)
	{
		assert(running);
		if (address.GetAddress() == 0)
			return false;
		//unsigned char packet[size+4];
		unsigned char* packet = (unsigned char*)malloc((size + 4) * sizeof(unsigned char));
		packet[0] = (unsigned char)(protocolId >> 24);
		packet[1] = (unsigned char)((protocolId >> 16) & 0xFF);
		packet[2] = (unsigned char)((protocolId >> 8) & 0xFF);
		packet[3] = (unsigned char)((protocolId) & 0xFF);
		memcpy(&packet[4], data, size);
		bool res =  socket.Send(address, packet, size + 4);
		//free(packet);
		return res;
	}

	int Connection::ReceivePacket(unsigned char data[], int size)
	{
		assert(running);
		//unsigned char packet[size+4];
		unsigned char* packet = (unsigned char*)malloc((size + 4) * sizeof(unsigned char));
		Address sender;
		int bytes_read = socket.Receive(sender, packet, size + 4);
		if (bytes_read == 0)
			return 0;
		if (bytes_read <= 4)
			return 0;
		if (packet[0] != (unsigned char)(protocolId >> 24) ||
			packet[1] != (unsigned char)((protocolId >> 16) & 0xFF) ||
			packet[2] != (unsigned char)((protocolId >> 8) & 0xFF) ||
			packet[3] != (unsigned char)(protocolId & 0xFF))
			return 0;

		if (mode == Server && !IsConnected())
		{
			printf("server accepts connection from client %d.%d.%d.%d:%d\n",
				sender.GetA(), sender.GetB(), sender.GetC(), sender.GetD(), sender.GetPort());
			state = Connected;
			address = sender;
			OnConnect();
		}
		if (sender == address)
		{
			if (mode == Client && state == Connecting)
			{
				printf("client completes connection with server\n");
				state = Connected;
				OnConnect();
			}
			timeoutAccumulator = 0.0f;
			memcpy(data, &packet[4], bytes_read - 4);
			return bytes_read - 4;
		}
		return 0;
	}

	int Connection::GetHeaderSize() const
	{
		return 4;
	}

	void Connection::ClearData()
	{
		state = Disconnected;
		timeoutAccumulator = 0.0f;
		address = Address(); //fix this
	}

	//reliable connection
	ReliableConnection::ReliableConnection(unsigned int protocolId, float timeout, unsigned int max_sequence)
		: Connection(protocolId, timeout), reliabilitySystem(max_sequence)
	{
		ClearData();
	}

	ReliableConnection::~ReliableConnection()
	{
		if (IsRunning())
			Stop();
	}

	bool ReliableConnection::SendPacket(const unsigned char data[], int size)
	{
		const int header = 12;
		unsigned char* packet = (unsigned char*)malloc((header + size) * sizeof(unsigned char));
		unsigned int seq = reliabilitySystem.GetLocalSequence();
		unsigned int ack = reliabilitySystem.GetRemoteSequence();
		unsigned int ack_bits = reliabilitySystem.GenerateAckBits();
		WriteHeader(packet, seq, ack, ack_bits);
		memcpy(packet + header, data, size);
		if (!Connection::SendPacket(packet, size + header))
			return false;
		reliabilitySystem.PacketSent(size);
		//free(packet);
		return true;
	}

	int ReliableConnection::ReceivePacket(unsigned char data[], int size)
	{
		const int header = 12;
		if (size <= header)
			return false;
		//unsigned char packet[header+size];
		unsigned char* packet = (unsigned char*)malloc((header + size) * sizeof(unsigned char));
		int received_bytes = Connection::ReceivePacket(packet, size + header);
		if (received_bytes == 0)
			return false;
		if (received_bytes < header)
			return false;
		unsigned int packet_sequence = 0;
		unsigned int packet_ack = 0;
		unsigned int packet_ack_bits = 0;
		ReadHeader(packet, packet_sequence, packet_ack, packet_ack_bits);
		reliabilitySystem.PacketReceived(packet_sequence, received_bytes - header);
		reliabilitySystem.ProcessAck(packet_ack, packet_ack_bits);
		memcpy(data, packet + header, received_bytes - header);
		return received_bytes - header;
	}

	void ReliableConnection::Update(float deltaTime)
	{
		Connection::Update(deltaTime);
		reliabilitySystem.Update(deltaTime);
	}

	int ReliableConnection::GetHeaderSize() const
	{
		return Connection::GetHeaderSize() + reliabilitySystem.GetHeaderSize();
	}

	ReliabilitySystem& ReliableConnection::GetReliabilitySystem()
	{
		return reliabilitySystem;
	}

	void ReliableConnection::WriteInteger(unsigned char* data, unsigned int value)
	{
		data[0] = (unsigned char)(value >> 24);
		data[1] = (unsigned char)((value >> 16) & 0xFF);
		data[2] = (unsigned char)((value >> 8) & 0xFF);
		data[3] = (unsigned char)(value & 0xFF);
	}

	void ReliableConnection::WriteHeader(unsigned char* header, unsigned int sequence, unsigned int ack, unsigned int ack_bits)
	{
		WriteInteger(header, sequence);
		WriteInteger(header + 4, ack);
		WriteInteger(header + 8, ack_bits);
	}

	void ReliableConnection::ReadInteger(const unsigned char* data, unsigned int& value)
	{
		value = (((unsigned int)data[0] << 24) | ((unsigned int)data[1] << 16) |
			((unsigned int)data[2] << 8) | ((unsigned int)data[3]));
	}

	void ReliableConnection::ReadHeader(const unsigned char* header, unsigned int& sequence, unsigned int& ack, unsigned int& ack_bits)
	{
		ReadInteger(header, sequence);
		ReadInteger(header + 4, ack);
		ReadInteger(header + 8, ack_bits);
	}

	void ReliableConnection::OnStop()
	{
		ClearData();
	}

	void ReliableConnection::OnDisconnect()
	{
		ClearData();
	}

	void ReliableConnection::ClearData()
	{
		reliabilitySystem.Reset();
	}
}