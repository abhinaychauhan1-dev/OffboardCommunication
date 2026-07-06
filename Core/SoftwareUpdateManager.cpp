/**
 * @file SoftwareUpdateManager.cpp
 * @brief Implements application update checks, version comparison, and download handling.
 *
 */

#include "SoftwareUpdateManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>
#include <QVersionNumber>

SoftwareUpdateManager::SoftwareUpdateManager(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    const QString appVersion = QCoreApplication::applicationVersion().trimmed();
    m_currentVersion = appVersion.isEmpty() ? QStringLiteral("1.0.0") : appVersion;

    const QString envFeed = qEnvironmentVariable("OFFBOARD_UPDATE_FEED").trimmed();
    m_feedUrl = envFeed.isEmpty()
                ? QStringLiteral("https://example.com/offboard-communication/releases/latest.json")
                : envFeed;

    setStatusMessage(QStringLiteral("Ready to check for updates."));
}

QString SoftwareUpdateManager::currentVersion() const { return m_currentVersion; }
QString SoftwareUpdateManager::latestVersion() const { return m_latestVersion; }
QString SoftwareUpdateManager::releaseNotes() const { return m_releaseNotes; }
QString SoftwareUpdateManager::feedUrl() const { return m_feedUrl; }
QString SoftwareUpdateManager::downloadedFilePath() const { return m_downloadedFilePath; }
QString SoftwareUpdateManager::statusMessage() const { return m_statusMessage; }
bool SoftwareUpdateManager::updateAvailable() const { return m_updateAvailable; }
bool SoftwareUpdateManager::checking() const { return m_checking; }
bool SoftwareUpdateManager::downloading() const { return m_downloading; }
double SoftwareUpdateManager::downloadProgress() const { return m_downloadProgress; }

void SoftwareUpdateManager::setFeedUrl(const QString& url)
{
    const QString trimmed = url.trimmed();
    if (trimmed.isEmpty() || trimmed == m_feedUrl) {
        return;
    }

    m_feedUrl = trimmed;
    setStatusMessage(QStringLiteral("Update feed URL updated."));
    emit updateStateChanged();
}

void SoftwareUpdateManager::checkForUpdates()
{
    if (m_checking || m_downloading) {
        return;
    }

    const QUrl url(m_feedUrl);
    if (!url.isValid() || url.scheme().isEmpty()) {
        setStatusMessage(QStringLiteral("Invalid update feed URL."));
        emit updateStateChanged();
        return;
    }

    m_checking = true;
    m_updateAvailable = false;
    m_latestVersion.clear();
    m_releaseNotes.clear();
    m_downloadUrl.clear();
    m_downloadedFilePath.clear();
    setStatusMessage(QStringLiteral("Checking for updates..."));
    emit updateStateChanged();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("OffboardCommunication/%1").arg(m_currentVersion));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    m_checkReply = m_networkManager->get(request);
    connect(m_checkReply, &QNetworkReply::finished,
            this, &SoftwareUpdateManager::onCheckRequestFinished);
}

void SoftwareUpdateManager::downloadUpdate()
{
    if (m_downloading || m_checking) {
        return;
    }

    if (!m_updateAvailable || m_downloadUrl.isEmpty()) {
        setStatusMessage(QStringLiteral("No downloadable update available."));
        emit updateStateChanged();
        return;
    }

    QUrl packageUrl(m_downloadUrl);
    if (!packageUrl.isValid() || packageUrl.scheme().isEmpty()) {
        setStatusMessage(QStringLiteral("Invalid update package URL."));
        emit updateStateChanged();
        return;
    }

    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadDir.isEmpty()) {
        downloadDir = QDir::currentPath();
    }

    QDir dir(downloadDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    QString fileName = QFileInfo(packageUrl.path()).fileName();
    if (fileName.isEmpty()) {
        fileName = QStringLiteral("OffboardCommunication-%1-update.pkg").arg(m_latestVersion);
    }

    const QString targetPath = dir.filePath(fileName);

    m_downloadFile.reset(new QSaveFile(targetPath));
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        setStatusMessage(QStringLiteral("Unable to create update package file."));
        m_downloadFile.reset();
        emit updateStateChanged();
        return;
    }

    m_downloading = true;
    m_downloadProgress = 0.0;
    m_downloadedFilePath.clear();
    setStatusMessage(QStringLiteral("Downloading update package..."));
    emit updateStateChanged();

    QNetworkRequest request(packageUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("OffboardCommunication/%1").arg(m_currentVersion));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);

    m_downloadReply = m_networkManager->get(request);
    connect(m_downloadReply, &QNetworkReply::readyRead,
            this, &SoftwareUpdateManager::onDownloadReadyRead);
    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, &SoftwareUpdateManager::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished,
            this, &SoftwareUpdateManager::onDownloadFinished);
}

void SoftwareUpdateManager::onCheckRequestFinished()
{
    m_checking = false;

    if (!m_checkReply) {
        setStatusMessage(QStringLiteral("Update check failed unexpectedly."));
        emit updateStateChanged();
        return;
    }

    const QNetworkReply::NetworkError error = m_checkReply->error();
    const QByteArray payload = m_checkReply->readAll();

    if (error != QNetworkReply::NoError) {
        setStatusMessage(QStringLiteral("Update check failed: %1").arg(m_checkReply->errorString()));
        m_checkReply->deleteLater();
        m_checkReply = nullptr;
        emit updateStateChanged();
        return;
    }

    m_checkReply->deleteLater();
    m_checkReply = nullptr;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setStatusMessage(QStringLiteral("Update feed format is invalid."));
        emit updateStateChanged();
        return;
    }

    const QJsonObject object = document.object();
    m_latestVersion = object.value(QStringLiteral("version")).toString().trimmed();
    m_releaseNotes = object.value(QStringLiteral("notes")).toString().trimmed();
    m_downloadUrl = object.value(QStringLiteral("downloadUrl")).toString().trimmed();

    if (m_latestVersion.isEmpty()) {
        setStatusMessage(QStringLiteral("Update feed does not include a version."));
        emit updateStateChanged();
        return;
    }

    const QVersionNumber currentVer = QVersionNumber::fromString(m_currentVersion);
    const QVersionNumber latestVer = QVersionNumber::fromString(m_latestVersion);

    if (!latestVer.isNull() && !currentVer.isNull()
        && QVersionNumber::compare(latestVer, currentVer) > 0) {
        m_updateAvailable = true;
        setStatusMessage(QStringLiteral("Update %1 is available.").arg(m_latestVersion));
    } else {
        m_updateAvailable = false;
        setStatusMessage(QStringLiteral("You already have the latest version."));
    }

    emit updateStateChanged();
}

void SoftwareUpdateManager::onDownloadReadyRead()
{
    if (!m_downloadReply || !m_downloadFile) {
        return;
    }

    const QByteArray chunk = m_downloadReply->readAll();
    if (chunk.isEmpty()) {
        return;
    }

    if (m_downloadFile->write(chunk) != chunk.size()) {
        setStatusMessage(QStringLiteral("Write failure during update download."));
        resetDownloadState();
        emit updateStateChanged();
    }
}

void SoftwareUpdateManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal <= 0) {
        m_downloadProgress = 0.0;
    } else {
        m_downloadProgress = static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal);
    }
    emit updateStateChanged();
}

void SoftwareUpdateManager::onDownloadFinished()
{
    if (!m_downloadReply) {
        resetDownloadState();
        emit updateStateChanged();
        return;
    }

    const QNetworkReply::NetworkError error = m_downloadReply->error();
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;

    if (error != QNetworkReply::NoError) {
        setStatusMessage(QStringLiteral("Download failed."));
        resetDownloadState();
        emit updateStateChanged();
        return;
    }

    if (!m_downloadFile || !m_downloadFile->commit()) {
        setStatusMessage(QStringLiteral("Unable to finalize downloaded update package."));
        resetDownloadState();
        emit updateStateChanged();
        return;
    }

    m_downloading = false;
    m_downloadProgress = 1.0;
    m_downloadedFilePath = m_downloadFile->fileName();
    m_downloadFile.reset();
    setStatusMessage(QStringLiteral("Update package downloaded."));
    emit updateStateChanged();
}

void SoftwareUpdateManager::setStatusMessage(const QString& message)
{
    m_statusMessage = message;
}

void SoftwareUpdateManager::resetDownloadState()
{
    m_downloading = false;
    m_downloadProgress = 0.0;

    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }

    if (m_downloadFile) {
        m_downloadFile->cancelWriting();
        m_downloadFile.reset();
    }
}
