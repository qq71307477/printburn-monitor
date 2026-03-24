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
#include <QString>

class PersonalHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalHomePage(QWidget *parent = nullptr);

    // 设置当前用户，加载用户相关数据
    void setCurrentUser(const QString &userId);

signals:
    void navigateToPage(const QString &pageName);

private:
    void setupUI();
    void setupUserInfoSection();
    void setupQuickAccessSection();
    void setupRecentActivitiesSection();
    void setupStatisticsSection();

    // 数据加载方法
    void loadUserInfo();
    void loadStatistics();
    void loadRecentActivities();

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

    // Current user data
    QString m_currentUserId;
    int m_currentUserIdInt;
};

#endif // PERSONALHOMEPAGE_H