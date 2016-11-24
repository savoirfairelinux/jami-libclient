#include "accountMedia.h"





/*******************************************************************************
*                                                                             *
*                                  Getters                                    *
*                                                                             *
******************************************************************************/

bool AccountMedia::isVideoEnabled() const
{
	return d_ptr->accountDetail(DRing::Account::ConfProperties::Video::ENABLED) IS_TRUE;
}

int AccountMedia::videoPortMax() const
{
	return d_ptr->accountDetail(DRing::Account::ConfProperties::Video::PORT_MAX).toInt();
}

int AccountMedia::videoPortMin() const
{
	return d_ptr->accountDetail(DRing::Account::ConfProperties::Video::PORT_MIN).toInt();
}

int AccountMedia::audioPortMin() const
{
	return d_ptr->accountDetail(DRing::Account::ConfProperties::Audio::PORT_MIN).toInt();
}

int AccountMedia::audioPortMax() const
{
	return d_ptr->accountDetail(DRing::Account::ConfProperties::Audio::PORT_MAX).toInt();
}

///Return if the ringtone are enabled
bool AccountMedia::isRingtoneEnabled() const
{
	return (d_ptr->accountDetail(DRing::Account::ConfProperties::Ringtone::ENABLED) IS_TRUE);
}

///Return the account ringtone path
QString AccountMedia::ringtonePath() const
{
	return d_ptr->accountDetail(DRing::Account::ConfProperties::Ringtone::PATH);
}



#define CAST(item) static_cast<int>(item)
QVariant AccountMedia::roleData(int role) const
{
	switch (role) {
	case CAST(AccountMedia::Role::IsVideoEnabled):
		return isVideoEnabled();
	case CAST(AccountMedia::Role::VideoPortMax):
		return videoPortMax();
	case CAST(AccountMedia::Role::VideoPortMin):
		return videoPortMin();
	case CAST(AccountMedia::Role::AudioPortMin):
		return audioPortMin();
	case CAST(AccountMedia::Role::AudioPortMax):
		return audioPortMax();
	case CAST(AccountMedia::Role::RingtonePath):
		return ringtonePath();
	case CAST(AccountMedia::Role::RingtoneEnabled):
		return isRingtoneEnabled();
	default:
		return QVariant();
	}
	return QVariant();
}
#undef CAST






/*****************************************************************************
*                                                                           *
*                                  Setters                                  *
*                                                                           *
****************************************************************************/

///Use video by default when available
void AccountMedia::setVideoEnabled(bool enable)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Video::ENABLED, (enable)TO_BOOL);
}

/**Set the maximum audio port
* This can be used when some routers without UPnP support open a narrow range
* of ports to allow the stream to go through.
*/
void AccountMedia::setAudioPortMax(int port)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Audio::PORT_MAX, QString::number(port));
}

/**Set the minimum audio port
* This can be used when some routers without UPnP support open a narrow range
* of ports to allow the stream to go through.
*/
void AccountMedia::setAudioPortMin(int port)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Audio::PORT_MIN, QString::number(port));
}

/**Set the maximum video port
* This can be used when some routers without UPnP support open a narrow range
* of ports to allow the stream to go through.
*/
void AccountMedia::setVideoPortMax(int port)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Video::PORT_MAX, QString::number(port));
}

/**Set the minimum video port
* This can be used when some routers without UPnP support open a narrow range
* of ports to allow the stream to go through.
*/
void AccountMedia::setVideoPortMin(int port)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Video::PORT_MIN, QString::number(port));
}

///Set the ringtone path, it have to be a valid absolute path
void AccountMedia::setRingtonePath(const QString& detail)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Ringtone::PATH, detail);
}

///Set if custom ringtone are enabled
void AccountMedia::setRingtoneEnabled(bool detail)
{
	d_ptr->setAccountProperty(DRing::Account::ConfProperties::Ringtone::ENABLED, (detail)TO_BOOL);
}

#define CAST(item) static_cast<int>(item)
void AccountMedia::setRoleData(int role, const QVariant& value)
{
	switch (role) {
	case CAST(AccountMedia::Role::IsVideoEnabled):
		setVideoEnabled(value.toBool());
		break;
	case CAST(AccountMedia::Role::VideoPortMax):
		setVideoPortMax(value.toInt());
		break;
	case CAST(AccountMedia::Role::VideoPortMin):
		setVideoPortMin(value.toInt());
		break;
	case CAST(AccountMedia::Role::AudioPortMin):
		setAudioPortMin(value.toInt());
		break;
	case CAST(AccountMedia::Role::AudioPortMax):
		setAudioPortMax(value.toInt());
		break;
	case CAST(AccountMedia::Role::RingtonePath):
		setRingtonePath(value.toString());
		break;
	case CAST(AccountMedia::Role::RingtoneEnabled):
		setRingtoneEnabled(value.toBool());
		break;
	}
}
#undef CAST