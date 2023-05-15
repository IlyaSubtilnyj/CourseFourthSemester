#pragma once
/*
	Simple Network Library from "Networking for Game Programmers"
	http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/

#ifndef NET_H
#define NET_H

// platform detection

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#include <WS2tcpip.h>
#include <winsock2.h>
#pragma comment( lib, "ws2_32.lib" )
#pragma warning(disable: 4996)

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#else

#error unknown platform!

#endif

#include <assert.h>
#include <vector>
#include <map>
#include <stack>
#include <list>
#include <algorithm>
#include <functional>

namespace net
{

	void wait(float seconds);

	class Address
	{
	public:

		Address();

		Address(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port);

		Address(unsigned int address, unsigned short port);

		unsigned int GetAddress() const;
		unsigned char GetA() const;
		unsigned char GetB() const;
		unsigned char GetC() const;
		unsigned char GetD() const;
		unsigned short GetPort() const;

		bool operator == (const Address& other) const;

		bool operator != (const Address& other) const;

		bool operator < (const Address& other) const;

	private:

		unsigned int address;
		unsigned short port;
	};


	inline bool InitializeSockets()
	{
#if PLATFORM == PLATFORM_WINDOWS
		WSADATA WsaData;
		return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
#else
		return true;
#endif
	}

	inline void ShutdownSockets()
	{
#if PLATFORM == PLATFORM_WINDOWS
		WSACleanup();
#endif
	}

	class Socket
	{
	public:

		Socket();
		~Socket();
		char* get_local_ip();
		bool Open(const char* port);
		void Close();
		bool IsOpen() const;
		bool Send(const Address& destination, const void* data, int size);
		int Receive(Address& sender, void* data, int size);

	private:

		int socket;
	};

	// connection

	// packet queue to store information about sent and received packets sorted in sequence order
	//  + we define ordering using the "sequence_more_recent" function, this works provided there is a large gap when sequence wrap occurs

	struct PacketData
	{
		unsigned int sequence;			// packet sequence number
		float time;					    // time offset since packet was sent or received (depending on context)
		int size;						// packet size in bytes
	};

	inline bool sequence_more_recent(unsigned int s1, unsigned int s2, unsigned int max_sequence)
	{
		return (s1 > s2) && (s1 - s2 <= max_sequence / 2) || (s2 > s1) && (s2 - s1 > max_sequence / 2);
	}

	class PacketQueue : public std::list<PacketData>
	{
	public:

		bool exists(unsigned int sequence);

		void insert_sorted(const PacketData& p, unsigned int max_sequence);

		void verify_sorted(unsigned int max_sequence);
	};

	// reliability system to support reliable connection
	//  + manages sent, received, pending ack and acked packet queues
	//  + separated out from reliable connection because it is quite complex and i want to unit test it!

	class ReliabilitySystem
	{
	public:

		static void SetMaxSeq(unsigned int max_sequence = 0xFFFFFFFF);
		ReliabilitySystem(unsigned int max_sequence = 0xFFFFFFFF);

		void Reset();

		void PacketSent(int size);

		void PacketReceived(unsigned int sequence, int size);

		unsigned int GenerateAckBits();

		void ProcessAck(unsigned int ack, unsigned int ack_bits);

		void Update(float deltaTime);

		void Validate();

		// utility functions

		static bool sequence_more_recent(unsigned int s1, unsigned int s2, unsigned int max_sequence);

		static int bit_index_for_sequence(unsigned int sequence, unsigned int ack, unsigned int max_sequence);

		static unsigned int generate_ack_bits(unsigned int ack, const PacketQueue& received_queue, unsigned int max_sequence);
		static void process_ack(unsigned int ack, unsigned int ack_bits,
			PacketQueue& pending_ack_queue, PacketQueue& acked_queue,
			std::vector<unsigned int>& acks, unsigned int& acked_packets,
			float& rtt, unsigned int max_sequence);

		// data accessors

		unsigned int GetLocalSequence() const;

		unsigned int GetRemoteSequence() const;

		unsigned int GetMaxSequence() const;

		void GetAcks(unsigned int** acks, int& count);
		unsigned int GetSentPackets() const;

		unsigned int GetReceivedPackets() const;

		unsigned int GetLostPackets() const;
		unsigned int GetAckedPackets() const;

		float GetSentBandwidth() const;
		float GetAckedBandwidth() const;
		float GetRoundTripTime() const;

		int GetHeaderSize() const;

	protected:

		void AdvanceQueueTime(float deltaTime);

		void UpdateQueues();

		void UpdateStats();

	private:

		static unsigned int max_sequence;			// maximum sequence value before wrap around (used to test sequence wrap at low # values)
		unsigned int local_sequence;		// local sequence number for most recently sent packet
		unsigned int remote_sequence;		// remote sequence number for most recently received packet

		unsigned int sent_packets;			// total number of packets sent
		unsigned int recv_packets;			// total number of packets received
		unsigned int lost_packets;			// total number of packets lost
		unsigned int acked_packets;			// total number of packets acked

		float sent_bandwidth;				// approximate sent bandwidth over the last second
		float acked_bandwidth;				// approximate acked bandwidth over the last second
		float rtt;							// estimated round trip time
		float rtt_maximum;					// maximum expected round trip time (hard coded to one second for the moment)

		std::vector<unsigned int> acks;		// acked packets from last set of packet receives. cleared each update!

		PacketQueue sentQueue;				// sent packets used to calculate sent bandwidth (kept until rtt_maximum)
		PacketQueue pendingAckQueue;		// sent packets which have not been acked yet (kept until rtt_maximum * 2 )
		PacketQueue receivedQueue;			// received packets for determining acks to send (kept up to most recent recv sequence - 32)
		PacketQueue ackedQueue;				// acked packets (kept until rtt_maximum * 2)
	};

	class Connection
	{
	public:

		enum Mode
		{
			None,
			Client,
			Server
		};

		Connection(unsigned int protocolId, float timeout);

		virtual ~Connection();

		bool Start(const char* port);

		void Stop();

		bool IsRunning() const;

		void Listen();

		void Connect(const Address& address);

		bool IsConnecting() const;

		bool ConnectFailed() const;

		bool IsConnected() const;
		bool IsListening() const;
		bool IsDisconnected() const;

		Mode GetMode() const;

		virtual void Update(float deltaTime);

		virtual bool SendPacket(const unsigned char data[], int size);

		virtual int ReceivePacket(unsigned char data[], int size);

		int GetHeaderSize() const;

	protected:

		virtual void OnStart() {}
		virtual void OnStop() {}
		virtual void OnConnect() {}
		virtual void OnDisconnect() {}

	private:

		void ClearData();

		enum State
		{
			Disconnected,
			Listening,
			Connecting,
			ConnectFail,
			Connected
		};

	public:
		unsigned int protocolId;
		float timeout;

	public:
		bool running;
		Mode mode;
		State state;
		Socket socket;
		float timeoutAccumulator;

		Address address;
	};

	// connection with reliability (seq/ack)

	class ReliableConnection : public Connection
	{
	public:

		ReliableConnection(unsigned int protocolId, float timeout, unsigned int max_sequence = 0xFFFFFFFF);

		~ReliableConnection();

		// overriden functions from "Connection"

		virtual bool SendPacket(const unsigned char data[], int size);

		virtual int ReceivePacket(unsigned char data[], int size);

		void Update(float deltaTime);

		int GetHeaderSize() const;

		ReliabilitySystem& GetReliabilitySystem();

	protected:

		void WriteInteger(unsigned char* data, unsigned int value);

		void WriteHeader(unsigned char* header, unsigned int sequence, unsigned int ack, unsigned int ack_bits);

		void ReadInteger(const unsigned char* data, unsigned int& value);

		void ReadHeader(const unsigned char* header, unsigned int& sequence, unsigned int& ack, unsigned int& ack_bits);

		virtual void OnStop();

		virtual void OnDisconnect();

	private:

		void ClearData();

		ReliabilitySystem reliabilitySystem;	// reliability system: manages sequence numbers and acks, tracks network stats etc.
	};

}
#endif