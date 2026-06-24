#pragma once

#include<string>
#include<vector>
#include<future>
#include "types/agent_types.h"
#include <functional>
#include<any>
#include<iostream>


namespace CodeGraph {
	
	class EventEmitter {
	public:
		void on(const std::string& event, std::function<void(std::any)> callback) {
			std::lock_guard<std::mutex> lock(listenerMutex_);
			listeners_[event].push_back(callback);
		}
		void emit(const std::string& event, std::any data = {}) {
			std::lock_guard<std::mutex>lock(listenerMutex_);
			auto it = listeners_.find(event);
			if (it != listeners_.end()) {
				for (auto& cb : it->second) {
					cb(data);
				}
			}
		}
		virtual ~EventEmitter() = default;
		
	private:
		std::unordered_map<std::string, std::vector<std::function<void(std::any)>>> listeners_;
		std::mutex listenerMutex_;

	}; 
}


