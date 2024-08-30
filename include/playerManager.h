#ifndef __PLAYER_MANAGER_H__
#define __PLAYER_MANAGER_H__

#include <unordered_map>
#include <functional>
#include <shared_mutex>
#include <memory>
#include <thread>

#include "Common.h"
#include "playerDisplay.h"
#include "playerWidget.h"


using namespace std;

class PlayerManager:public QObject
{
	Q_OBJECT
public:
	explicit PlayerManager();
	~PlayerManager();

	int open_file(QString filename, WId widId);
	void play_stop();
	bool playStatus_toggle();
	void seek_forward();
	void seek_back();

	inline std::shared_ptr<PlayerDisplay> getDisplayPtr() const {
		return displayPtr;
	}


private:
	shared_ptr<PlayerDisplay>displayPtr;


};





#endif // __PLAYER_MANAGER_H__

