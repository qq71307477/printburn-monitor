#ifndef PERSONALHOMEPAGE_H
#define PERSONALHOMEPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QGroupBox>
#include <QProgressBar>

class PersonalHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalHomePage(QWidget *parent = nullptr);

private:
    void setupUI();
    void setupUserInfoSection();
    void setupQuickAccessSection();
    void setupRecentActivitiesSection();
    void setupStatisticsSection();

    // UI Components
    QVBoxLayout *m_layout;

    // User Info
    QGroupBox *m_userInfoGroup;
    QLabel *m_avatarLabel;
    QLabel *m_nameLabel;
    QLabel *m_roleLabel;
    QLabel *m_departmentLabel;

    // Quick Access
    QGroupBox *m_quickAccessGroup;
    QWidget *m_quickAccessWidget;

    // Recent Activities
    QGroupBox *m_recentActivitiesGroup;
    QTableWidget *m_activitiesTable;

    // Statistics
    QGroupBox *m_statisticsGroup;
    QGridLayout *m_statsLayout;
    QProgressBar *m_printProgress;
    QProgressBar *m_burnProgress;
    QLabel *m_printCountLabel;
    QLabel *m_burnCountLabel;
    QLabel *m_approvalCountLabel;
    QLabel *m_pendingApprovalLabel;
};

#endif // PERSONALHOMEPAGE_H