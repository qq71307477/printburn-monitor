#include "proxy_approver_repository.h"
#include <QVariant>
#include <QDebug>

// Static member initialization
DatabaseManager* ProxyApproverRepository::default_db_manager_ = nullptr;

ProxyApproverRepository::ProxyApproverRepository() : db_manager_(default_db_manager_) {}

ProxyApproverRepository::ProxyApproverRepository(DatabaseManager* db_manager) : db_manager_(db_manager) {}

bool ProxyApproverRepository::create_table() {
    if (!db_manager_) return false;

    QString sql = R"(
        CREATE TABLE IF NOT EXISTS proxy_approvers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_user_id INTEGER NOT NULL,
            proxy_user_id INTEGER NOT NULL,
            min_security_level_id INTEGER DEFAULT 0,
            max_security_level_id INTEGER DEFAULT 9999,
            task_type INTEGER DEFAULT 2,
            start_date DATE,
            end_date DATE,
            enabled INTEGER DEFAULT 1,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (owner_user_id) REFERENCES users(id),
            FOREIGN KEY (proxy_user_id) REFERENCES users(id),
            FOREIGN KEY (min_security_level_id) REFERENCES security_levels(id),
            FOREIGN KEY (max_security_level_id) REFERENCES security_levels(id)
        );

        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_owner ON proxy_approvers(owner_user_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_proxy ON proxy_approvers(proxy_user_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_min_security ON proxy_approvers(min_security_level_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_max_security ON proxy_approvers(max_security_level_id);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_task_type ON proxy_approvers(task_type);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_enabled ON proxy_approvers(enabled);
        CREATE INDEX IF NOT EXISTS idx_proxy_approvers_dates ON proxy_approvers(start_date, end_date);
    )";
    return db_manager_->execute_query(sql);
}

ProxyApprover ProxyApproverRepository::findById(int id) {
    ProxyApprover proxy;
    if (!db_manager_) return proxy;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                  "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        proxy.id = query.value(0).toInt();
        proxy.owner_user_id = query.value(1).toInt();
        proxy.proxy_user_id = query.value(2).toInt();
        proxy.min_security_level_id = query.value(3).toInt();
        proxy.max_security_level_id = query.value(4).toInt();
        proxy.task_type = query.value(5).toInt();
        proxy.start_date = query.value(6).toDate();
        proxy.end_date = query.value(7).toDate();
        proxy.enabled = query.value(8).toBool();
        proxy.created_at = query.value(9).toDateTime();
        proxy.updated_at = query.value(10).toDateTime();
    }
    return proxy;
}

QList<ProxyApprover> ProxyApproverRepository::findAll() {
    QList<ProxyApprover> proxies;
    if (!db_manager_) return proxies;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                   "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers ORDER BY id")) {
        while (query.next()) {
            ProxyApprover proxy;
            proxy.id = query.value(0).toInt();
            proxy.owner_user_id = query.value(1).toInt();
            proxy.proxy_user_id = query.value(2).toInt();
            proxy.min_security_level_id = query.value(3).toInt();
            proxy.max_security_level_id = query.value(4).toInt();
            proxy.task_type = query.value(5).toInt();
            proxy.start_date = query.value(6).toDate();
            proxy.end_date = query.value(7).toDate();
            proxy.enabled = query.value(8).toBool();
            proxy.created_at = query.value(9).toDateTime();
            proxy.updated_at = query.value(10).toDateTime();
            proxies.append(proxy);
        }
    }
    return proxies;
}

bool ProxyApproverRepository::create(ProxyApprover& proxy) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("INSERT INTO proxy_approvers (owner_user_id, proxy_user_id, min_security_level_id, "
                  "max_security_level_id, task_type, start_date, end_date, enabled) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(proxy.owner_user_id);
    query.addBindValue(proxy.proxy_user_id);
    query.addBindValue(proxy.min_security_level_id);
    query.addBindValue(proxy.max_security_level_id);
    query.addBindValue(proxy.task_type);
    query.addBindValue(proxy.start_date.isNull() ? QVariant() : proxy.start_date);
    query.addBindValue(proxy.end_date.isNull() ? QVariant() : proxy.end_date);
    query.addBindValue(proxy.enabled ? 1 : 0);

    if (query.exec()) {
        proxy.id = query.lastInsertId().toInt();
        return true;
    }
    qWarning() << "Failed to create proxy approver:" << query.lastError().text();
    return false;
}

bool ProxyApproverRepository::update(const ProxyApprover& proxy) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("UPDATE proxy_approvers SET owner_user_id = ?, proxy_user_id = ?, "
                  "min_security_level_id = ?, max_security_level_id = ?, task_type = ?, "
                  "start_date = ?, end_date = ?, enabled = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    query.addBindValue(proxy.owner_user_id);
    query.addBindValue(proxy.proxy_user_id);
    query.addBindValue(proxy.min_security_level_id);
    query.addBindValue(proxy.max_security_level_id);
    query.addBindValue(proxy.task_type);
    query.addBindValue(proxy.start_date.isNull() ? QVariant() : proxy.start_date);
    query.addBindValue(proxy.end_date.isNull() ? QVariant() : proxy.end_date);
    query.addBindValue(proxy.enabled ? 1 : 0);
    query.addBindValue(proxy.id);

    if (!query.exec()) {
        qWarning() << "Failed to update proxy approver:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool ProxyApproverRepository::deleteById(int id) {
    if (!db_manager_) return false;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("DELETE FROM proxy_approvers WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool ProxyApproverRepository::save(ProxyApprover& proxy) {
    if (proxy.id > 0) {
        return update(proxy);
    }
    return create(proxy);
}

QList<ProxyApprover> ProxyApproverRepository::findByOwnerUserId(int ownerUserId) {
    QList<ProxyApprover> proxies;
    if (!db_manager_) return proxies;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                  "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers "
                  "WHERE owner_user_id = ? ORDER BY id");
    query.addBindValue(ownerUserId);

    if (query.exec()) {
        while (query.next()) {
            ProxyApprover proxy;
            proxy.id = query.value(0).toInt();
            proxy.owner_user_id = query.value(1).toInt();
            proxy.proxy_user_id = query.value(2).toInt();
            proxy.min_security_level_id = query.value(3).toInt();
            proxy.max_security_level_id = query.value(4).toInt();
            proxy.task_type = query.value(5).toInt();
            proxy.start_date = query.value(6).toDate();
            proxy.end_date = query.value(7).toDate();
            proxy.enabled = query.value(8).toBool();
            proxy.created_at = query.value(9).toDateTime();
            proxy.updated_at = query.value(10).toDateTime();
            proxies.append(proxy);
        }
    }
    return proxies;
}

QList<ProxyApprover> ProxyApproverRepository::findByProxyUserId(int proxyUserId) {
    QList<ProxyApprover> proxies;
    if (!db_manager_) return proxies;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                  "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers "
                  "WHERE proxy_user_id = ? ORDER BY id");
    query.addBindValue(proxyUserId);

    if (query.exec()) {
        while (query.next()) {
            ProxyApprover proxy;
            proxy.id = query.value(0).toInt();
            proxy.owner_user_id = query.value(1).toInt();
            proxy.proxy_user_id = query.value(2).toInt();
            proxy.min_security_level_id = query.value(3).toInt();
            proxy.max_security_level_id = query.value(4).toInt();
            proxy.task_type = query.value(5).toInt();
            proxy.start_date = query.value(6).toDate();
            proxy.end_date = query.value(7).toDate();
            proxy.enabled = query.value(8).toBool();
            proxy.created_at = query.value(9).toDateTime();
            proxy.updated_at = query.value(10).toDateTime();
            proxies.append(proxy);
        }
    }
    return proxies;
}

QList<ProxyApprover> ProxyApproverRepository::findActiveByOwner(int ownerUserId, int taskType, int securityLevelId) {
    QList<ProxyApprover> proxies;
    if (!db_manager_) return proxies;

    QDate today = QDate::currentDate();

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                  "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers "
                  "WHERE owner_user_id = ? AND enabled = 1 "
                  "AND (task_type = 2 OR task_type = ?) "
                  "AND min_security_level_id <= ? AND max_security_level_id >= ? "
                  "AND (start_date IS NULL OR start_date <= ?) "
                  "AND (end_date IS NULL OR end_date >= ?) "
                  "ORDER BY id");
    query.addBindValue(ownerUserId);
    query.addBindValue(taskType);
    query.addBindValue(securityLevelId);
    query.addBindValue(securityLevelId);
    query.addBindValue(today);
    query.addBindValue(today);

    if (query.exec()) {
        while (query.next()) {
            ProxyApprover proxy;
            proxy.id = query.value(0).toInt();
            proxy.owner_user_id = query.value(1).toInt();
            proxy.proxy_user_id = query.value(2).toInt();
            proxy.min_security_level_id = query.value(3).toInt();
            proxy.max_security_level_id = query.value(4).toInt();
            proxy.task_type = query.value(5).toInt();
            proxy.start_date = query.value(6).toDate();
            proxy.end_date = query.value(7).toDate();
            proxy.enabled = query.value(8).toBool();
            proxy.created_at = query.value(9).toDateTime();
            proxy.updated_at = query.value(10).toDateTime();
            proxies.append(proxy);
        }
    }
    return proxies;
}

QList<ProxyApprover> ProxyApproverRepository::findEnabled() {
    QList<ProxyApprover> proxies;
    if (!db_manager_) return proxies;

    QSqlQuery query(db_manager_->get_connection());
    if (query.exec("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                   "task_type, start_date, end_date, enabled, created_at, updated_at "
                   "FROM proxy_approvers WHERE enabled = 1 ORDER BY id")) {
        while (query.next()) {
            ProxyApprover proxy;
            proxy.id = query.value(0).toInt();
            proxy.owner_user_id = query.value(1).toInt();
            proxy.proxy_user_id = query.value(2).toInt();
            proxy.min_security_level_id = query.value(3).toInt();
            proxy.max_security_level_id = query.value(4).toInt();
            proxy.task_type = query.value(5).toInt();
            proxy.start_date = query.value(6).toDate();
            proxy.end_date = query.value(7).toDate();
            proxy.enabled = query.value(8).toBool();
            proxy.created_at = query.value(9).toDateTime();
            proxy.updated_at = query.value(10).toDateTime();
            proxies.append(proxy);
        }
    }
    return proxies;
}

bool ProxyApproverRepository::isActive(int proxyId) {
    if (!db_manager_) return false;

    QDate today = QDate::currentDate();

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT COUNT(*) FROM proxy_approvers "
                  "WHERE id = ? AND enabled = 1 "
                  "AND (start_date IS NULL OR start_date <= ?) "
                  "AND (end_date IS NULL OR end_date >= ?)");
    query.addBindValue(proxyId);
    query.addBindValue(today);
    query.addBindValue(today);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

bool ProxyApproverRepository::hasConflict(const ProxyApprover& proxy, int excludeId) {
    if (!db_manager_) return false;

    // Check if the same owner already has a proxy with overlapping settings
    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT COUNT(*) FROM proxy_approvers "
                  "WHERE owner_user_id = ? AND proxy_user_id = ? "
                  "AND (task_type = 2 OR task_type = ? OR ? = 2) "
                  "AND min_security_level_id <= ? AND max_security_level_id >= ? "
                  "AND enabled = 1 "
                  "AND id != ?");
    query.addBindValue(proxy.owner_user_id);
    query.addBindValue(proxy.proxy_user_id);
    query.addBindValue(proxy.task_type);
    query.addBindValue(proxy.task_type);
    query.addBindValue(proxy.max_security_level_id);
    query.addBindValue(proxy.min_security_level_id);
    query.addBindValue(excludeId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}

// Legacy snake_case methods for backward compatibility
std::unique_ptr<ProxyApprover> ProxyApproverRepository::find_by_id(int id) {
    if (!db_manager_) return nullptr;

    QSqlQuery query(db_manager_->get_connection());
    query.prepare("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                  "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        auto proxy = std::make_unique<ProxyApprover>();
        proxy->id = query.value(0).toInt();
        proxy->owner_user_id = query.value(1).toInt();
        proxy->proxy_user_id = query.value(2).toInt();
        proxy->min_security_level_id = query.value(3).toInt();
        proxy->max_security_level_id = query.value(4).toInt();
        proxy->task_type = query.value(5).toInt();
        proxy->start_date = query.value(6).toDate();
        proxy->end_date = query.value(7).toDate();
        proxy->enabled = query.value(8).toBool();
        proxy->created_at = query.value(9).toDateTime();
        proxy->updated_at = query.value(10).toDateTime();
        return proxy;
    }
    return nullptr;
}

std::vector<std::unique_ptr<ProxyApprover>> ProxyApproverRepository::find_all() {
    std::vector<std::unique_ptr<ProxyApprover>> proxies;
    if (!db_manager_) return proxies;

    QSqlQuery query(db_manager_->get_connection());

    if (query.exec("SELECT id, owner_user_id, proxy_user_id, min_security_level_id, max_security_level_id, "
                   "task_type, start_date, end_date, enabled, created_at, updated_at FROM proxy_approvers ORDER BY id")) {
        while (query.next()) {
            auto proxy = std::make_unique<ProxyApprover>();
            proxy->id = query.value(0).toInt();
            proxy->owner_user_id = query.value(1).toInt();
            proxy->proxy_user_id = query.value(2).toInt();
            proxy->min_security_level_id = query.value(3).toInt();
            proxy->max_security_level_id = query.value(4).toInt();
            proxy->task_type = query.value(5).toInt();
            proxy->start_date = query.value(6).toDate();
            proxy->end_date = query.value(7).toDate();
            proxy->enabled = query.value(8).toBool();
            proxy->created_at = query.value(9).toDateTime();
            proxy->updated_at = query.value(10).toDateTime();
            proxies.push_back(std::move(proxy));
        }
    }
    return proxies;
}