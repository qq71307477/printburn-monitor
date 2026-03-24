#include "SecurityLevelService.h"
#include "src/common/repository/security_level_repository.h"
#include <QDateTime>
#include <mutex>

// Static instance
static std::once_flag serviceOnceFlag;
static SecurityLevelService* serviceInstance = nullptr;

SecurityLevelService& SecurityLevelService::getInstance()
{
    std::call_once(serviceOnceFlag, []() {
        serviceInstance = new SecurityLevelService();
    });
    return *serviceInstance;
}

SecurityLevelService::SecurityLevelService()
{
}

SecurityLevelService::~SecurityLevelService()
{
}

bool SecurityLevelService::createLevel(const SecurityLevel& level)
{
    // Validate level data
    QString error = validateLevel(level);
    if (!error.isEmpty()) {
        return false;
    }

    SecurityLevelRepository repo;
    SecurityLevel newLevel = level;
    newLevel.setActive(true);
    newLevel.setCreatedAt(QDateTime::currentDateTime());
    newLevel.setUpdatedAt(QDateTime::currentDateTime());

    return repo.create(newLevel);
}

bool SecurityLevelService::updateLevel(const SecurityLevel& level)
{
    if (level.getId() <= 0) {
        return false;
    }

    // Validate level data
    QString error = validateLevel(level);
    if (!error.isEmpty()) {
        return false;
    }

    // Check if level exists
    SecurityLevelRepository repo;
    SecurityLevel existingLevel = repo.findById(level.getId());
    if (existingLevel.getId() <= 0) {
        return false;
    }

    SecurityLevel updatedLevel = level;
    updatedLevel.setUpdatedAt(QDateTime::currentDateTime());

    return repo.update(updatedLevel);
}

bool SecurityLevelService::deleteLevel(int id)
{
    if (id <= 0) {
        return false;
    }

    SecurityLevelRepository repo;
    SecurityLevel level = repo.findById(id);
    if (level.getId() <= 0) {
        return false;
    }

    // Use soft delete by deactivating instead of hard delete
    return deactivateLevel(id);
}

SecurityLevel SecurityLevelService::getLevel(int id) const
{
    SecurityLevelRepository repo;
    return repo.findById(id);
}

SecurityLevel SecurityLevelService::getLevelByCode(const QString& levelCode) const
{
    SecurityLevelRepository repo;
    return repo.findByLevelCode(levelCode);
}

QList<SecurityLevel> SecurityLevelService::getAllLevels() const
{
    SecurityLevelRepository repo;
    return repo.findAll();
}

QList<SecurityLevel> SecurityLevelService::getActiveLevels() const
{
    SecurityLevelRepository repo;
    return repo.findActive();
}

QString SecurityLevelService::validateLevel(const SecurityLevel& level) const
{
    // Check level_code
    if (level.getLevelCode().isEmpty()) {
        return QString("密级编号不能为空");
    }

    // Check level_name
    if (level.getLevelName().isEmpty()) {
        return QString("密级名称不能为空");
    }

    // Check retention_days - must be positive
    if (level.getRetentionDays() <= 0) {
        return QString("保存天数必须大于0");
    }

    // Check timeout_minutes - must be positive
    if (level.getTimeoutMinutes() <= 0) {
        return QString("超时分钟数必须大于0");
    }

    // Check for duplicate level_code (only for new records or if code changed)
    SecurityLevelRepository repo;
    SecurityLevel existingLevel = repo.findByLevelCode(level.getLevelCode());
    if (existingLevel.getId() > 0 && existingLevel.getId() != level.getId()) {
        return QString("密级编号 '%1' 已存在").arg(level.getLevelCode());
    }

    return QString(); // Empty string means validation passed
}

bool SecurityLevelService::deactivateLevel(int id)
{
    if (id <= 0) {
        return false;
    }

    SecurityLevelRepository repo;
    SecurityLevel level = repo.findById(id);
    if (level.getId() <= 0) {
        return false;
    }

    level.setActive(false);
    level.setUpdatedAt(QDateTime::currentDateTime());

    return repo.update(level);
}

bool SecurityLevelService::activateLevel(int id)
{
    if (id <= 0) {
        return false;
    }

    SecurityLevelRepository repo;
    SecurityLevel level = repo.findById(id);
    if (level.getId() <= 0) {
        return false;
    }

    level.setActive(true);
    level.setUpdatedAt(QDateTime::currentDateTime());

    return repo.update(level);
}
