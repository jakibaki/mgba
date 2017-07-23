/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef QGBA_CORE_CONTROLLER
#define QGBA_CORE_CONTROLLER

#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QSize>

#include "VFileDevice.h"

#include <functional>
#include <memory>

#include <mgba/core/core.h>
#include <mgba/core/interface.h>
#include <mgba/core/thread.h>
#include <mgba/core/tile-cache.h>

struct mCore;

namespace QGBA {

class ConfigController;
class InputController;
class LogController;
class MultiplayerController;
class Override;

class CoreController : public QObject {
Q_OBJECT

public:
	static const bool VIDEO_SYNC = false;
	static const bool AUDIO_SYNC = true;

	class Interrupter {
	public:
		Interrupter(CoreController*, bool fromThread = false);
		Interrupter(std::shared_ptr<CoreController>, bool fromThread = false);
		Interrupter(const Interrupter&);
		~Interrupter();

	private:
		CoreController* m_parent;
	};

	CoreController(mCore* core, QObject* parent = nullptr);
	~CoreController();

	mCoreThread* thread() { return &m_threadContext; }

	color_t* drawContext();

	bool isPaused();

	mPlatform platform() const;
	QSize screenDimensions() const;

	void loadConfig(ConfigController*);

	mCheatDevice* cheatDevice() { return m_threadContext.core->cheatDevice(m_threadContext.core); }

#ifdef USE_DEBUGGERS
	mDebugger* debugger() { return m_threadContext.core->debugger; }
	void setDebugger(mDebugger*);
#endif

	void setMultiplayerController(MultiplayerController*);
	void clearMultiplayerController();
	MultiplayerController* multiplayerController() { return m_multiplayer; }

	mTileCache* tileCache();
	int stateSlot() const { return m_stateSlot; }

	void setOverride(std::unique_ptr<Override> override);
	Override* override() { return m_override.get(); }

	void setInputController(InputController*);
	void setLogger(LogController*);

public slots:
	void start();
	void stop();
	void reset();
	void setPaused(bool paused);
	void frameAdvance();
	void setSync(bool enable);

	void setRewinding(bool);
	void rewind(int count = 0);

	void setFastForward(bool);
	void forceFastForward(bool);

	void loadState(int slot = 0);
	void saveState(int slot = 0);
	void loadBackupState();
	void saveBackupState();

	void loadSave(const QString&, bool temporary);
	void loadPatch(const QString&);
	void replaceGame(const QString&);
	void yankPak();

#ifdef USE_PNG
	void screenshot();
#endif

	void setRealTime();
	void setFixedTime(const QDateTime& time);
	void setFakeEpoch(const QDateTime& time);

	void importSharkport(const QString& path);
	void exportSharkport(const QString& path);

	void setAVStream(mAVStream*);
	void clearAVStream();

	void clearOverride();

	void startVideoLog(const QString& path);
	void endVideoLog();

signals:
	void started();
	void paused();
	void unpaused();
	void stopping();
	void crashed(const QString& errorMessage);
	void failed();
	void frameAvailable();
	void stateLoaded();
	void rewound();

	void rewindChanged(bool);
	void fastForwardChanged(bool);

	void unimplementedBiosCall(int);
	void statusPosted(const QString& message);
	void logPosted(int level, int category, const QString& log);

private:
	void updateKeys();
	void finishFrame();

	void updateFastForward();

	mCoreThread m_threadContext{};

	bool m_patched = false;

	QByteArray m_buffers[2];
	QByteArray* m_activeBuffer;
	QByteArray* m_completeBuffer = nullptr;

	std::unique_ptr<mTileCache> m_tileCache;
	std::unique_ptr<Override> m_override;

	QList<std::function<void()>> m_resetActions;
	QList<std::function<void()>> m_frameActions;
	QMutex m_mutex;

	VFileDevice m_backupLoadState;
	QByteArray m_backupSaveState{nullptr};
	int m_stateSlot = 1;
	int m_loadStateFlags;
	int m_saveStateFlags;

	bool m_audioSync = AUDIO_SYNC;
	bool m_videoSync = VIDEO_SYNC;

	int m_fastForward = false;
	int m_fastForwardForced = false;
	float m_fastForwardRatio = -1.f;
	float m_fpsTarget;

	InputController* m_inputController = nullptr;
	LogController* m_log = nullptr;
	MultiplayerController* m_multiplayer = nullptr;

	mVideoLogContext* m_vl = nullptr;
	VFile* m_vlVf = nullptr;
};

}

#endif
