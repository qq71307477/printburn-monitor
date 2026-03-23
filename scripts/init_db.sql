-- Database initialization script for print etching security monitoring system
-- Using SQLite syntax

PRAGMA foreign_keys = ON;

-- Roles table
CREATE TABLE IF NOT EXISTS roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    permissions TEXT,  -- JSON string containing permissions
    is_active BOOLEAN DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Departments table
CREATE TABLE IF NOT EXISTS departments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    manager_id INTEGER,
    parent_department_id INTEGER,
    is_active BOOLEAN DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (manager_id) REFERENCES users(id),
    FOREIGN KEY (parent_department_id) REFERENCES departments(id)
);

-- Users table
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    role_id INTEGER NOT NULL,
    department_id INTEGER,
    first_name TEXT,
    last_name TEXT,
    phone TEXT,
    is_active BOOLEAN DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (role_id) REFERENCES roles(id),
    FOREIGN KEY (department_id) REFERENCES departments(id)
);

-- Devices table
CREATE TABLE IF NOT EXISTS devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    device_id TEXT NOT NULL UNIQUE,  -- Unique identifier for the device
    device_type TEXT,                -- printer, scanner, etc.
    ip_address TEXT,
    location TEXT,
    assigned_user_id INTEGER,        -- Currently assigned user
    status TEXT DEFAULT 'available', -- available, in_use, maintenance, offline
    specifications TEXT,             -- Technical specs
    is_monitored BOOLEAN DEFAULT 1,  -- Whether the device is under security monitoring
    last_seen DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (assigned_user_id) REFERENCES users(id)
);

-- Tasks table
CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT,
    assigned_user_id INTEGER NOT NULL,
    created_by_user_id INTEGER NOT NULL,
    device_id INTEGER,
    status TEXT DEFAULT 'pending',   -- pending, in_progress, completed, cancelled
    priority TEXT DEFAULT 'medium',  -- low, medium, high, critical
    due_date DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    completed_at DATETIME,
    FOREIGN KEY (assigned_user_id) REFERENCES users(id),
    FOREIGN KEY (created_by_user_id) REFERENCES users(id),
    FOREIGN KEY (device_id) REFERENCES devices(id)
);

-- Indexes for better performance
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_role_id ON users(role_id);
CREATE INDEX IF NOT EXISTS idx_users_department_id ON users(department_id);
CREATE INDEX IF NOT EXISTS idx_devices_device_id ON devices(device_id);
CREATE INDEX IF NOT EXISTS idx_devices_assigned_user ON devices(assigned_user_id);
CREATE INDEX IF NOT EXISTS idx_devices_status ON devices(status);
CREATE INDEX IF NOT EXISTS idx_tasks_assigned_user ON tasks(assigned_user_id);
CREATE INDEX IF NOT EXISTS idx_tasks_created_by ON tasks(created_by_user_id);
CREATE INDEX IF NOT EXISTS idx_tasks_device_id ON tasks(device_id);
CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);
CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date);

-- Insert default roles
INSERT OR IGNORE INTO roles (name, description, permissions) VALUES
('admin', 'System administrator with full access', '{"users": "crud", "devices": "crud", "tasks": "crud", "reports": "read"}'),
('manager', 'Department manager with limited admin rights', '{"users": "crud", "devices": "crud", "tasks": "crud", "reports": "read"}'),
('user', 'Regular user with basic access', '{"users": "r", "devices": "r", "tasks": "crud", "reports": "read"}'),
('auditor', 'Audit user with read-only access', '{"users": "r", "devices": "r", "tasks": "r", "reports": "read"}');

-- Insert default departments
INSERT OR IGNORE INTO departments (name, description) VALUES
('IT Department', 'Information Technology Department'),
('HR Department', 'Human Resources Department'),
('Finance Department', 'Finance Department'),
('Security Department', 'Security Operations Department');

-- Insert default admin user (password hash is for "admin123")
INSERT OR IGNORE INTO users (username, password_hash, email, role_id, department_id, first_name, last_name, is_active) VALUES
('admin', 'pbkdf2:sha256:260000$...', 'admin@example.com', 1, 1, 'System', 'Administrator', 1);