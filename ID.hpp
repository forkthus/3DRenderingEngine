#pragma once

#include <queue>

using std::queue;

class ID {
public:
	ID() {
		nextID = 0;
	}

	unsigned int getID() {
		if (availableIDs.empty()) {
			return nextID++;
		}
		else {
			int id = availableIDs.front();
			availableIDs.pop();
			return id;
		}
	}

	void releaseID(int id) {
		availableIDs.push(id);
	}

private:
	unsigned int nextID;
	queue<int> availableIDs;
};