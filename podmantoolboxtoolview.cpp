// podmantoolboxtoolview.cpp

#include "podmantoolboxtoolview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <KToolBar>
#include <QProcess>
#include <QTextEdit>
#include <QRegularExpression>

PodmanToolboxToolView::PodmanToolboxToolView(QWidget *parent)
    : QWidget(parent),
      m_stackedWidget(new QStackedWidget(this)),
      m_containerListPage(new QWidget(this)),
      m_containerDetailsPage(new QWidget(this)),
      m_statusLabel(new QLabel(this)),
      m_containerComboBox(new QComboBox(this)),
      m_startButton(new QPushButton(i18n("Start"), this)),
      m_stopButton(new QPushButton(i18n("Stop"), this)),
      m_execButton(new QPushButton(i18n("Exec"), this)),
      m_connectButton(new QPushButton(i18n("Connect"), this)),
      m_outputTextEdit(new QTextEdit(this))
{
    // Set up container list page
    QVBoxLayout *containerListPageLayout = new QVBoxLayout(m_containerListPage);
    containerListPageLayout->addWidget(new QLabel(i18n("Available Containers:"), this));
    containerListPageLayout->addWidget(m_containerComboBox);

    // Set up container details page
    QVBoxLayout *containerDetailsPageLayout = new QVBoxLayout(m_containerDetailsPage);
    containerDetailsPageLayout->addWidget(new QLabel(i18n("Container Details:"), this));
    containerDetailsPageLayout->addWidget(m_statusLabel);
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_execButton);
    buttonLayout->addWidget(m_connectButton); // Add connect button
    containerDetailsPageLayout->addLayout(buttonLayout);
    containerDetailsPageLayout->addWidget(m_outputTextEdit);

    // Add pages to stacked widget
    m_stackedWidget->addWidget(m_containerListPage);
    m_stackedWidget->addWidget(m_containerDetailsPage);

    // Set up main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_stackedWidget);

    // Connect signals and slots
    connect(m_startButton, &QPushButton::clicked, this, &PodmanToolboxToolView::startContainer);
    connect(m_stopButton, &QPushButton::clicked, this, &PodmanToolboxToolView::stopContainer);
    connect(m_execButton, &QPushButton::clicked, this, &PodmanToolboxToolView::execInContainer);
    connect(m_connectButton, &QPushButton::clicked, this, &PodmanToolboxToolView::onConnectButtonClicked);

    refreshContainers();
}

PodmanToolboxToolView::~PodmanToolboxToolView()
{
}

void PodmanToolboxToolView::setupToolBars(KActionCollection *actionCollection)
{
    KToolBar *toolBar = new KToolBar(this);
    toolBar->addAction(actionCollection->action(QStringLiteral("start_container")));
    toolBar->addAction(actionCollection->action(QStringLiteral("stop_container")));
    toolBar->addAction(actionCollection->action(QStringLiteral("exec_in_container")));
    toolBar->addAction(actionCollection->action(QStringLiteral("connect_to_container")));
}

void PodmanToolboxToolView::displayMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void PodmanToolboxToolView::displayOutput(const QString &output)
{
    m_outputTextEdit->append(output);
}

void PodmanToolboxToolView::displayError(const QString &error)
{
    m_outputTextEdit->append(QString("<font color=\"red\">%1</font>").arg(error));
}

QString PodmanToolboxToolView::getSelectedContainerName() const
{
    return m_containerComboBox->currentText();
}

void PodmanToolboxToolView::refreshContainers()
{
    m_containerComboBox->clear();

    QProcess process;
    process.start("distrobox-list", QStringList() << "-f" << "{{.Name}}"); // Use distrobox-list to get container names
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList containerNames = output.split("\n", Qt::SkipEmptyParts);

    m_containerComboBox->addItems(containerNames);
}

void PodmanToolboxToolView::startContainer()
{
    emit connectToContainer();
}

void PodmanToolboxToolView::stopContainer()
{
    emit connectToContainer();
}

void PodmanToolboxToolView::execInContainer()
{
    emit connectToContainer();
}

void PodmanToolboxToolView::onConnectButtonClicked()
