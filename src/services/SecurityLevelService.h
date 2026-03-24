#ifndef SECURITY_LEVEL_SERVICE_H
#define SECURITY_LEVEL_SERVICE_H

#include <QString>
#include <QList>
#include "models/security_level_model.h"

class SecurityLevelService
{
public:
    // Singleton pattern
    static SecurityLevelService& getInstance();

    // CRUD operations
    bool createLevel(const SecurityLevel& level);
    bool updateLevel(const SecurityLevel& level);
    bool deleteLevel(int id);
    SecurityLevel getLevel(int id) const;
    SecurityLevel getLevelByCode(const QString& levelCode) const;
    QList<SecurityLevel> getAllLevels() const;
    QList<SecurityLevel> getActiveLevels() const;

    // Validation
    QString validateLevel(const SecurityLevel& level) const;

    // Soft delete - deactivate level
    bool deactivateLevel(int id);

    // Activate level
    bool activateLevel(int id);

private:
    SecurityLevelService();
    ~SecurityLevelService();

    // Disallow copy and assignment
    SecurityLevelService(const SecurityLevelService&) = delete;
    SecurityLevelService& operator=(const SecurityLevelService&) = delete;
};

#endif // SECURITY_LEVEL_SERVICE_H
