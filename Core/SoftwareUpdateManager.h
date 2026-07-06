/**
 * @file SoftwareUpdateManager.h
 * @brief Declares the software update manager used by the simulator UI and backend.
 *
 * Developed by Abhinay Chauhan
 */

#ifndef SOFTWAREUPDATEMANAGER_H
#define SOFTWAREUPDATEMANAGER_H

#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QSaveFile>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class SoftwareUpdateManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY updateStateChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY updateStateChanged)
    Q_PROPERTY(QString feedUrl READ feedUrl NOTIFY updateStateChanged)
    Q_PROPERTY(QString downloadedFilePath READ downloadedFilePath NOTIFY updateStateChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY updateStateChanged)
    Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateStateChanged)
    Q_PROPERTY(bool checking READ checking NOTIFY updateStateChanged)
    Q_PROPERTY(bool downloading READ downloading NOTIFY updateStateChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY updateStateChanged)

public:
    explicit SoftwareUpdateManager(QObject* parent = nullptr);

    QString currentVersion() const;
    QString latestVersion() const;
    QString releaseNotes() const;
    QString feedUrl() const;
    QString downloadedFilePath() const;
    QString statusMessage() const;
    bool updateAvailable() const;
    bool checking() const;
    bool downloading() const;
    double downloadProgress() const;

    Q_INVOKABLE void setFeedUrl(const QString& url);
    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void downloadUpdate();

signals:
    void updateStateChanged();

private slots:
    void onCheckRequestFinished();
    void onDownloadReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    void setStatusMessage(const QString& message);
    void resetDownloadState();

    QString m_currentVersion;
    QString m_latestVersion;
    QString m_releaseNotes;
    QString m_feedUrl;
    QString m_downloadUrl;
    QString m_downloadedFilePath;
    QString m_statusMessage;

    bool m_updateAvailable = false;
    bool m_checking = false;
    bool m_downloading = false;
    double m_downloadProgress = 0.0;

    QNetworkAccessManager* m_networkManager = nullptr;
    QPointer<QNetworkReply> m_checkReply;
    QPointer<QNetworkReply> m_downloadReply;
    QScopedPointer<QSaveFile> m_downloadFile;
};

#endif
