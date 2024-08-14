#ifndef __PLAYER_CLOCK_H__
#define __PLAYER_CLOCK_H__

#include "avPacketList.h"

extern "C"
{
#include "libavutil/time.h"
#include "libavutil/mathematics.h"
}

class PlayerClock
{
public:
	PlayerClock();
	~PlayerClock();

	double get_clock();
	void set_clock_at(double pts, int serial, double time);
	void set_clock(double pts, int serial);
	void set_clock_speed(double speed);
	void init_clock(AvPacketList* list);
	void sync_clock_to_slave(PlayerClock* slave);

public:
	inline void set_last_update(int lastUpdate) { m_last_updated = lastUpdate; }
	inline int get_last_update() const { return m_last_updated; }
	inline void set_serial(int serial) { m_serial = serial; }
	inline int get_serial() const { return m_serial; }
	inline void set_pause(int pause) { m_paused = pause; }
	inline int get_pause() const { return m_paused; }

private:
	double m_pts;           
	double m_pts_drift;     
	double m_last_updated;
	double m_speed;
	int m_serial;           
	int m_paused;
	int pQueue_serial;

	AvPacketList* pList;

};





#endif // __PLAYER_CLOCK_H__
