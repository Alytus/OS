#pragma once
#include <utility>
#include <condition_variable>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <queue>

using namespace std;

#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_
template <typename T>
class BufferedChannel
{
	mutex ch_lock;
	queue<T> channel;
	int buffer_size;
	condition_variable cv_send, cv_recv;
	bool send_flag = true, recv_flag = false;
	bool closed = false;
public:
	BufferedChannel() { }

	explicit BufferedChannel(int size) {
		buffer_size = size;
	}

	void Send(T value) {
		unique_lock<mutex> lock(ch_lock);

		if (!closed && channel.size() == buffer_size) {
			send_flag = false;

			while (!send_flag) {
				cv_send.wait(lock);
				if (closed)
					break;
			}
		}

		if (closed) {
			throw runtime_error("Channel is closed!");
		}

		channel.push(move(value));
		recv_flag = true;
		cv_recv.notify_one();
	}

	pair<T, bool> Recv() {
		unique_lock<mutex> lock(ch_lock);

		if (!closed && channel.empty()) {
			recv_flag = false;

			while (!recv_flag) {
				cv_recv.wait(lock);
				if (closed)
					break;
				if (channel.empty())
					recv_flag = false;
			}
		}

		if (closed && channel.empty()) {
			return make_pair(move(T()), false);
		}

		T value = move(channel.front());
		channel.pop();
		send_flag = true;
		cv_send.notify_one();
		return make_pair(move(value), true);
	}

	void Close() {
		lock_guard<mutex> lock(ch_lock);
		closed = true;
		cv_send.notify_all();
		cv_recv.notify_all();
	}

};
#endif //BUFFERED_CHANNEL_H_