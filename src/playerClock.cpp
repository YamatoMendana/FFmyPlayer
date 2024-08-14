#include "playerClock.h"

PlayerClock::PlayerClock()
{

}

PlayerClock::~PlayerClock()
{

}

double PlayerClock::get_clock()
{
	if (pList->get_serial() != m_serial)
		return NAN;
	if (m_paused) {
		return m_pts;
	}
	else {
		double time = av_gettime_relative() / 1000000.0;
		return m_pts_drift + time - (time - m_last_updated) * (1.0 - m_speed);
	}
}

void PlayerClock::set_clock_at(double pts, int serial, double time) 
{
	m_pts = pts;
	m_last_updated = time;
	m_pts_drift = pts - time;
	m_serial = serial;
}

void PlayerClock::set_clock(double pts, int serial)
{
	double time = av_gettime_relative() / 1000000.0;
	set_clock_at(pts, serial, time);
}

void PlayerClock::set_clock_speed(double speed)
{
	set_clock(get_clock(), m_serial);
	m_speed = speed;
}

void PlayerClock::init_clock(AvPacketList* list)
{
	m_speed = 1.0;
	m_paused = 0;
	pList = list;
	set_clock(NAN, -1);
}

void PlayerClock::sync_clock_to_slave(PlayerClock* slave)
{
	double clock = get_clock();
	double slave_clock = slave->get_clock();
	if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		set_clock(slave_clock, slave->m_serial);
}

