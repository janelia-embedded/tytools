/*
 * ty, a collection of GUI and command-line tools to manage Teensy devices
 *
 * Distributed under the MIT license (see LICENSE.txt or http://opensource.org/licenses/MIT)
 * Copyright (c) 2015 Niels Martignène <niels.martignene@gmail.com>
 */

#ifndef BOARD_HH
#define BOARD_HH

#include <QMutex>
#include <QStringList>
#include <QTextCodec>
#include <QTextDecoder>
#include <QTextDocument>
#include <QThread>
#include <QTimer>

#include <memory>
#include <vector>

#include "ty/board.h"
#include "database.hh"
#include "descriptor_notifier.hh"
#include "firmware.hh"
#include "ty/monitor.h"
#include "task.hh"

struct BoardInterfaceInfo {
    QString name;
    QString path;

    uint16_t capabilities;
    uint8_t number;
    bool open;
};

class Board : public QObject, public std::enable_shared_from_this<Board> {
    Q_OBJECT

    DatabaseInterface db_;

    ty_board *board_;

    bool serial_attach_ = false;
    ty_board_interface *serial_iface_ = nullptr;
    DescriptorNotifier serial_notifier_;
    QTextCodec *serial_codec_;
    std::unique_ptr<QTextDecoder> serial_decoder_;
    QMutex serial_lock_;
    char serial_buf_[262144];
    size_t serial_buf_len_ = 0;
    QTextDocument serial_document_;

    QTimer error_timer_;

    QString firmware_;
    bool reset_after_;
    QString serial_codec_name_;
    bool clear_on_reset_;

    QString status_firmware_;
    QStringList recent_firmwares_;

    ty_pool *pool_ = nullptr;

    TaskInterface running_task_;
    TaskWatcher task_watcher_;

public:
    static std::shared_ptr<Board> createBoard(ty_board *board);
    virtual ~Board();

    void setDatabase(DatabaseInterface db) { db_ = db; }
    DatabaseInterface database() const { return db_; }
    void loadSettings();

    ty_board *board() const { return board_; }

    bool matchesTag(const QString &id);

    ty_board_state state() const;
    uint16_t capabilities() const;

    const ty_board_model *model() const;
    QString modelName() const;

    QString tag() const;
    QString id() const;
    QString location() const;
    uint64_t serialNumber() const;

    std::vector<BoardInterfaceInfo> interfaces() const;

    bool isRunning() const;
    bool uploadAvailable() const;
    bool resetAvailable() const;
    bool rebootAvailable() const;
    bool serialAvailable() const;
    bool serialOpen() const { return serial_iface_; }
    bool errorOccured() const;

    QString statusIconFileName() const;
    QString statusText() const;

    QString firmware() const { return firmware_; }
    QStringList recentFirmwares() const { return recent_firmwares_; }
    bool resetAfter() const { return reset_after_; }
    QString serialCodecName() const { return serial_codec_name_; }
    QTextCodec *serialCodec() const { return serial_codec_; }
    bool clearOnReset() const { return clear_on_reset_; }
    unsigned int scrollBackLimit() const { return serial_document_.maximumBlockCount(); }
    bool attachMonitor() const { return serial_attach_; }

    QTextDocument &serialDocument() { return serial_document_; }
    void appendToSerialDocument(const QString& s);

    static QStringList makeCapabilityList(uint16_t capabilities);
    static QString makeCapabilityString(uint16_t capabilities, QString empty_str = QString());

    TaskInterface upload(const QString &filename = QString());
    TaskInterface upload(const std::vector<std::shared_ptr<Firmware>> &fws);
    TaskInterface upload(const std::vector<std::shared_ptr<Firmware>> &fws, bool reset_after);
    TaskInterface reset();
    TaskInterface reboot();

    bool sendSerial(const QByteArray &buf);
    bool sendSerial(const QString &s);

    TaskInterface runningTask() const { return running_task_; }

public slots:
    void setTag(const QString &tag);

    void setFirmware(const QString &firmware);
    void clearRecentFirmwares();
    void setResetAfter(bool reset_after);
    void setSerialCodecName(QString codec_name);
    void setClearOnReset(bool clear_on_reset);
    void setScrollBackLimit(unsigned int limit);
    void setAttachMonitor(bool attach_monitor);

    TaskInterface startUpload(const QString &filename = QString());
    TaskInterface startUpload(const std::vector<std::shared_ptr<Firmware>> &fws);
    TaskInterface startUpload(const std::vector<std::shared_ptr<Firmware>> &fws, bool reset_after);
    TaskInterface startReset();
    TaskInterface startReboot();

    void notifyLog(ty_log_level level, const QString &msg);

signals:
    void infoChanged();
    void settingsChanged();
    void interfacesChanged();
    void statusChanged();

    void dropped();

private slots:
    void serialReceived(ty_descriptor desc);
    void updateSerialDocument();

    void notifyFinished(bool success, std::shared_ptr<void> result);
    void notifyProgress(const QString &action, unsigned int value, unsigned int max);

private:
    Board(ty_board *board, QObject *parent = nullptr);

    void setThreadPool(ty_pool *pool) { pool_ = pool; }

    void refreshBoard();
    bool openSerialInterface();
    void closeSerialInterface();

    void addUploadedFirmware(ty_firmware *fw);

    TaskInterface watchTask(TaskInterface task);

    friend class Monitor;
};

#endif
