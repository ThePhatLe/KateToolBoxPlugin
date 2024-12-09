// podmantoolboxtoolview.h

#ifndef PODMANTOOLBOXTOOLVIEW_H
#define PODMANTOOLBOXTOOLVIEW_H

#include <QWidget>
#include <QStackedWidget>
#include <KActionCollection>

class QLabel;
class QComboBox;
class QPushButton;
class QTextEdit;

class PodmanToolboxToolView : public QWidget
{
    Q_OBJECT

public:
    explicit PodmanToolboxToolView(QWidget *parent = nullptr);
    ~PodmanToolboxToolView() override;

    void setupToolBars(KActionCollection *actionCollection);
    void displayMessage(const QString &message);
    void displayOutput(const QString &output);
    void displayError(const QString &error);
    QString getSelectedContainerName() const;
    void refreshContainers();

signals:
    void connectToContainer();

private slots:
    void startContainer();
    void stopContainer();
    void execInContainer();
    void onConnectButtonClicked();

private:
    QStackedWidget *m_stackedWidget;
    QWidget *m_containerListPage;
    QWidget *m_containerDetailsPage;

    QLabel *m_statusLabel;
    QComboBox *m_containerComboBox;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_execButton;
    QPushButton *m_connectButton;
    QTextEdit *m_outputTextEdit;
};

#endif // PODMANTOOLBOXTOOLVIEW_H
