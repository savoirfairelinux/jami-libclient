


class AccountMedia {

//Properties
Q_PROPERTY(bool           videoEnabled                 READ isVideoEnabled                WRITE setVideoEnabled)
Q_PROPERTY(int            audioPortMax                 READ audioPortMax                  WRITE setAudioPortMax)
Q_PROPERTY(int            audioPortMin                 READ audioPortMin                  WRITE setAudioPortMin)
Q_PROPERTY(int            videoPortMax                 READ videoPortMax                  WRITE setVideoPortMax)
Q_PROPERTY(int            videoPortMin                 READ videoPortMin                  WRITE setVideoPortMin)

public:

	enum class Role {
		IsVideoEnabled,
		AudioPortMax,
		AudioPortMin,
		VideoPortMax,
		VideoPortMin,
		RingtonePath,
		RingtoneEnabled,
	};

	//Getters
	bool    isVideoEnabled() const;
	int     audioPortMax() const;
	int     audioPortMin() const;
	int     videoPortMax() const;
	int     videoPortMin() const;	
	QString ringtonePath() const;
	bool    isRingtoneEnabled() const;

	Q_INVOKABLE QVariant roleData(int role) const;


	//Setters
	void setVideoEnabled(bool enable);
	void setAudioPortMax(int port);
	void setAudioPortMin(int port);
	void setVideoPortMax(int port);
	void setVideoPortMin(int port);
	void setRingtonePath(const QString& detail);
	void setRingtoneEnabled(bool detail);

	void setRoleData(int role, const QVariant& value);

};
