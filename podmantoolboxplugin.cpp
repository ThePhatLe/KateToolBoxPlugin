// podmantoolboxplugin.cpp

#include "podmantoolboxplugin.h"
#include "podmantoolboxtoolview.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginLoader>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include <QProcess>
#include <QRegularExpression>
#include <QDir>

K_PLUGIN_FACTORY_WITH_JSON(PodmanToolboxPluginFactory, "podmantoolbox.json", registerPlugin<PodmanToolboxPlugin>();)

PodmanToolboxPlugin::PodmanToolboxPlugin(QObject *parent)
: KPlugin(parent),
m_actionCollection(new KActionCollection(this)),
m_process(new QProcess(this))
{
    // Set up about data
    KAboutData aboutData(QStringLiteral("podmantoolbox"),
                         i18n("Podman/Distrobox Integration"),
                         QStringLiteral("1.0.0"),
                         i18n("Integrates Podman and Distrobox with Kate"),
                         KAboutLicense::GPL,
                         i18n("Copyright (c) 2024"),
                         QStringLiteral(""),
                         QStringLiteral("https://example.com/podmantoolbox"),
                         QStringLiteral("submit@bugs.kde.org"));
    setAboutData(aboutData);

    // Create tool view
    m_toolView = new PodmanToolboxToolView(this);

    // Connect process signals
    connect(m_process, &QProcess::readyReadStandardOutput, this, &PodmanToolboxPlugin::readProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &PodmanToolboxPlugin::readProcessError);
    connect(m_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &PodmanToolboxPlugin::processFinished);

    // Add actions to the collection
    m_actionCollection->addAction(QStringLiteral("start_container"), this, &PodmanToolboxPlugin::startContainer);
    m_actionCollection->addAction(QStringLiteral("stop_container"), this, &PodmanToolboxPlugin::stopContainer);
    m_actionCollection->addAction(QStringLiteral("exec_in_container"), this, &PodmanToolboxPlugin::execInContainer);
    m_actionCollection->addAction(QStringLiteral("connect_to_container"), this, &PodmanToolboxPlugin::connectToContainer);

    // Set up action toolbars
    m_toolView->setupToolBars(m_actionCollection);
}

PodmanToolboxPlugin::~PodmanToolboxPlugin()
{
}

void PodmanToolboxPlugin::startContainer()
{
    QString containerName = m_toolView->getSelectedContainerName();
    if (containerName.isEmpty()) {
        KMessageBox::warning(nullptr, i18n("Please select a container."));
        return;
    }

    m_toolView->displayMessage(i18n("Starting container %1...", containerName));
    QStringList args;

    // Detect if running inside Flatpak
    if (QDir("/.flatpak-info").exists()) {
        // Use flatpak-spawn to execute distrobox inside the host
        args << "--host" << "distrobox-enter" << "-n" << containerName;
    } else {
        // Use distrobox directly (non-Flatpak)
        args << "-n" << containerName;
    }

    m_process->start("distrobox-enter", args);
}

void PodmanToolboxPlugin::stopContainer()
{
    QString containerName = m_toolView->getSelectedContainerName();
    if (containerName.isEmpty()) {
        KMessageBox::warning(nullptr, i18n("Please select a container."));
        return;
    }

    m_toolView->displayMessage(i18n("Stopping container %1...", containerName));
    QStringList args;

    // Detect if running inside Flatpak
    if (QDir("/.flatpak-info").exists()) {
        // Use flatpak-spawn to execute podman inside the host
        args << "--host" << "podman" << "container" << "stop" << containerName;
    } else {
        // Use podman directly (non-Flatpak)
        args << "container" << "stop" << containerName;
    }

    m_process->start("podman", args);
}

void PodmanToolboxPlugin::execInContainer()
{
    QString containerName = m_toolView->getSelectedContainerName();
    if (containerName.isEmpty()) {
        KMessageBox::warning(nullptr, i18n("Please select a container."));
        return;
    }

    m_toolView->displayMessage(i18n("Executing command in container %1...", containerName));
    QStringList args;

    // Detect if running inside Flatpak
    if (QDir("/.flatpak-info").exists()) {
        // Use flatpak-spawn to execute distrobox inside the host
        args << "--host" << "distrobox-enter" << "-n" << containerName << "bash"; // Replace "bash" with the desired command
    } else {
        // Use distrobox directly (non-Flatpak)
        args << "-n" << containerName << "bash"; // Replace "bash" with the desired command
    }

    m_process->start("distrobox-enter", args);
}

void PodmanToolboxPlugin::connectToContainer()
{
    QString containerName = m_toolView->getSelectedContainerName();
    if (containerName.isEmpty()) {
        KMessageBox::warning(nullptr, i18n("Please select a container."));
        return;
    }

    m_toolView->displayMessage(i18n("Connecting to container %1...", containerName));

    // Check if container is running (using podman)
    QProcess checkProcess;
    QStringList checkArgs;

    // Detect if running inside Flatpak
    if (QDir("/.flatpak-info").exists()) {
        // Use flatpak-spawn to execute podman inside the host
        checkArgs << "--host" << "podman" << "container" << "inspect" << "-f" << "{{.State.Running}}" << containerName;
        checkProcess.start("flatpak-spawn", checkArgs);
    } else {
        // Use podman directly (non-Flatpak)
        checkArgs << "container" << "inspect" << "-f" << "{{.State.Running}}" << containerName;
        checkProcess.start("podman", checkArgs);
    }

    checkProcess.waitForFinished();

    QString isRunning = checkProcess.readAllStandardOutput().trimmed();

    if (isRunning == "false") {
        // Start the container if it's not running
        m_toolView->displayMessage(i18n("Container not running. Starting..."));
        startContainer(); // Use the startContainer function to handle Flatpak vs non-Flatpak
    } else {
        // Connect to the running container using distrobox
        QStringList connectArgs;

        // Detect if running inside Flatpak
        if (QDir("/.flatpak-info").exists()) {
            // Use flatpak-spawn to execute distrobox inside the host
            connectArgs << "--host" << "distrobox-enter" << "-n" << containerName << "bash"; // Replace "bash" with desired shell
        } else {
            // Use distrobox directly (non-Flatpak)
            connectArgs << "-n" << containerName << "bash"; // Replace "bash" with desired shell
        }

        m_process->start("distrobox-enter", connectArgs);
    }
}

void PodmanToolboxPlugin::readProcessOutput()
{
    QByteArray output = m_process->readAllStandardOutput();
    m_toolView->displayOutput(QString::fromLocal8Bit(output));
}

void PodmanToolboxPlugin::readProcessError()
{
    QByteArray error = m_process->readAllStandardError();
    m_toolView->displayError(QString::fromLocal8Bit(error));
}

void PodmanToolboxPlugin::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        m_toolView->displayMessage(i18n("Command executed successfully."));
    } else {
        m_toolView->displayError(i18n("Command failed with exit code %1.", exitCode));
    }
    m_toolView->refreshContainers();
}


// Enhancement: Open files in the container
void PodmanToolboxPlugin::openFileInContainer(KTextEditor::Document* document)
{
    QString containerName = m_toolView->getSelectedContainerName();
    if (containerName.isEmpty()) {
        KMessageBox::warning(nullptr, i18n("Please select a container."));
        return;
    }

    QString filePath = document->url().toLocalFile();

    // Get the active editor and view
    KTextEditor::Editor *editor = qobject_cast<KTextEditor::Editor*>(document->parent());
    KTextEditor::View *view = editor ? editor->activeView() : nullptr;

    if (!view) {
        KMessageBox::error(nullptr, i18n("Cannot get active editor view."));
        return;
    }

    // Check if container is running, start if necessary
    if (!isContainerRunning(containerName)) {
        startContainer();
        // Wait for the container to start (this is a simplification, you might need a more robust way to wait for the container)
        m_process->waitForFinished();
    }

    // Open the file in the container using distrobox
    QStringList args;

    // Detect if running inside Flatpak
    if (QDir("/.flatpak-info").exists()) {
        // Use flatpak-spawn to execute distrobox inside the host
        args << "--host" << "distrobox-enter" << "-n" << containerName << "kate" << filePath;
    } else {
        // Use distrobox directly (non-Flatpak)
        args << "-n" << containerName << "kate" << filePath;
    }

    m_process->start("distrobox-enter", args);
}

bool PodmanToolboxPlugin::isContainerRunning(const QString &containerName)
{
    QProcess checkProcess;
    QStringList checkArgs;

    // Detect if running inside Flatpak
    if (QDir("/.flatpak-info").exists()) {
        // Use flatpak-spawn to execute podman inside the host
        checkArgs << "--host" << "podman" << "container" << "inspect" << "-f" << "{{.State.Running}}" << containerName;
        checkProcess.start("flatpak-spawn", checkArgs);
    } else {
        // Use podman directly (non-Flatpak)
        checkArgs << "container" << "inspect" << "-f" << "{{.State.Running}}" << containerName;
        checkProcess.start("podman", checkArgs);
    }

    checkProcess.waitForFinished();
    QString isRunning = checkProcess.readAllStandardOutput().trimmed();
    return isRunning == "true";
}
