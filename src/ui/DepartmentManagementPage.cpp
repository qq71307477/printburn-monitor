#include "DepartmentManagementPage.h"
#include "src/common/repository/department_repository.h"
#include "src/common/repository/user_repository.h"
#include <QHeaderView>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDialog>

// DepartmentEditDialog Implementation

DepartmentEditDialog::DepartmentEditDialog(QWidget* parent, const Department& dept)
    : QDialog(parent)
    , m_dept(dept)
    , m_isEdit(dept.id > 0)
    , m_nameEdit(nullptr)
    , m_descriptionEdit(nullptr)
    , m_parentCombo(nullptr)
{
    setWindowTitle(m_isEdit ? "编辑部门" : "新增部门");
    setModal(true);
    resize(400, 250);
    setupUI();
}

void DepartmentEditDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout();

    // Department Name
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setText(QString::fromStdString(m_dept.name));
    m_nameEdit->setPlaceholderText("输入部门名称");
    formLayout->addRow("部门名称:*", m_nameEdit);

    // Department Description
    m_descriptionEdit = new QLineEdit(this);
    m_descriptionEdit->setText(QString::fromStdString(m_dept.description));
    m_descriptionEdit->setPlaceholderText("输入部门描述（可选）");
    formLayout->addRow("部门描述:", m_descriptionEdit);

    // Parent Department
    m_parentCombo = new QComboBox(this);
    m_parentCombo->addItem("无上级部门", 0);
    loadParentDepartments();
    formLayout->addRow("上级组织:", m_parentCombo);

    mainLayout->addLayout(formLayout);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* saveButton = new QPushButton("保存", this);
    QPushButton* cancelButton = new QPushButton("取消", this);

    connect(saveButton, &QPushButton::clicked, this, &DepartmentEditDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DepartmentEditDialog::onCancelClicked);

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void DepartmentEditDialog::loadParentDepartments()
{
    DepartmentRepository repo;
    auto departments = repo.find_all();

    for (const auto& dept : departments) {
        // Skip self when editing
        if (m_isEdit && dept->id == m_dept.id) {
            continue;
        }

        // Skip if this would create a cycle (simple check: don't allow selecting children)
        if (m_isEdit && dept->parent_department_id == m_dept.id) {
            continue;
        }

        m_parentCombo->addItem(QString::fromStdString(dept->name), dept->id);
    }

    // Select current parent if editing
    if (m_isEdit && m_dept.parent_department_id > 0) {
        int index = m_parentCombo->findData(m_dept.parent_department_id);
        if (index >= 0) {
            m_parentCombo->setCurrentIndex(index);
        }
    }
}

void DepartmentEditDialog::onSaveClicked()
{
    // Validate input
    if (m_nameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "验证错误", "部门名称不能为空！");
        m_nameEdit->setFocus();
        return;
    }

    m_dept.name = m_nameEdit->text().trimmed().toStdString();
    m_dept.description = m_descriptionEdit->text().trimmed().toStdString();
    m_dept.parent_department_id = m_parentCombo->currentData().toInt();

    accept();
}

void DepartmentEditDialog::onCancelClicked()
{
    reject();
}

Department DepartmentEditDialog::getDepartment() const
{
    return m_dept;
}

// DepartmentManagementPage Implementation

DepartmentManagementPage::DepartmentManagementPage(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_deptTree(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_refreshButton(nullptr)
{
    setupUI();
    loadData();
}

void DepartmentManagementPage::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel("组织架构", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0px;");
    m_mainLayout->addWidget(m_titleLabel);

    // Description label
    QLabel* descLabel = new QLabel("管理部门组织架构，支持多级部门结构。拖拽可调整部门层级关系。", this);
    descLabel->setStyleSheet("color: #666; margin-bottom: 10px;");
    m_mainLayout->addWidget(descLabel);

    // Toolbar
    setupToolbar();

    // Tree
    setupTree();

    // Status label
    QLabel* statusLabel = new QLabel("提示: 双击部门节点可直接编辑，拖拽部门可调整层级关系。", this);
    statusLabel->setStyleSheet("color: #999; font-size: 12px;");
    m_mainLayout->addWidget(statusLabel);

    m_mainLayout->addStretch();
}

void DepartmentManagementPage::setupToolbar()
{
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->addStretch();

    m_addButton = new QPushButton("新增", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_refreshButton = new QPushButton("刷新", this);

    connect(m_addButton, &QPushButton::clicked, this, &DepartmentManagementPage::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &DepartmentManagementPage::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &DepartmentManagementPage::onDeleteClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &DepartmentManagementPage::onRefreshClicked);

    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addWidget(m_refreshButton);

    m_mainLayout->addLayout(toolbarLayout);
}

void DepartmentManagementPage::setupTree()
{
    m_deptTree = new QTreeWidget(this);
    m_deptTree->setHeaderLabels({"部门名称", "部门描述", "部门ID"});
    m_deptTree->header()->setStretchLastSection(false);
    m_deptTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_deptTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_deptTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_deptTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_deptTree->setDragEnabled(true);
    m_deptTree->setAcceptDrops(true);
    m_deptTree->setDragDropMode(QAbstractItemView::InternalMove);
    m_deptTree->setDropIndicatorShown(true);

    // Hide the ID column
    m_deptTree->setColumnHidden(2, true);

    connect(m_deptTree, &QTreeWidget::itemDoubleClicked,
            this, &DepartmentManagementPage::onTreeItemDoubleClicked);

    m_mainLayout->addWidget(m_deptTree);
}

void DepartmentManagementPage::loadData()
{
    DepartmentRepository repo;
    auto departments = repo.find_all();
    refreshTree(departments);
}

void DepartmentManagementPage::refreshTree(const std::vector<std::unique_ptr<Department>>& depts)
{
    m_deptTree->clear();

    // Build a map of parent_id -> children
    QMap<int, QVector<const Department*>> childrenMap;
    QMap<int, const Department*> deptMap;

    for (const auto& dept : depts) {
        deptMap[dept->id] = dept.get();
        childrenMap[dept->parent_department_id].append(dept.get());
    }

    // Recursively build tree
    std::function<void(QTreeWidgetItem*, const Department*)> addNode =
        [&](QTreeWidgetItem* parent, const Department* dept) {
        QTreeWidgetItem* item;
        if (parent) {
            item = new QTreeWidgetItem(parent);
        } else {
            item = new QTreeWidgetItem(m_deptTree);
        }

        item->setText(0, QString::fromStdString(dept->name));
        item->setText(1, QString::fromStdString(dept->description));
        item->setText(2, QString::number(dept->id));
        item->setData(0, Qt::UserRole, dept->id);

        // Add children
        if (childrenMap.contains(dept->id)) {
            for (const Department* child : childrenMap[dept->id]) {
                addNode(item, child);
            }
        }
    };

    // Add root level departments (parent_id = 0)
    if (childrenMap.contains(0)) {
        for (const Department* dept : childrenMap[0]) {
            addNode(nullptr, dept);
        }
    }

    // Expand all items
    m_deptTree->expandAll();
}

Department DepartmentManagementPage::getSelectedDepartment() const
{
    Department dept;

    QTreeWidgetItem* selectedItem = m_deptTree->currentItem();
    if (!selectedItem) {
        return dept;
    }

    int deptId = selectedItem->data(0, Qt::UserRole).toInt();

    DepartmentRepository repo;
    auto found = repo.find_by_id(deptId);
    if (found) {
        dept = *found;
    }

    return dept;
}

void DepartmentManagementPage::onAddClicked()
{
    // If a department is selected, pre-select it as parent
    QTreeWidgetItem* selectedItem = m_deptTree->currentItem();
    Department newDept;
    if (selectedItem) {
        newDept.parent_department_id = selectedItem->data(0, Qt::UserRole).toInt();
    }

    DepartmentEditDialog dialog(this, newDept);

    if (dialog.exec() == QDialog::Accepted) {
        Department dept = dialog.getDepartment();

        DepartmentRepository repo;
        if (repo.create(dept)) {
            QMessageBox::information(this, "成功", "部门添加成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "部门添加失败！");
        }
    }
}

void DepartmentManagementPage::onEditClicked()
{
    Department dept = getSelectedDepartment();
    if (dept.id <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的部门！");
        return;
    }

    DepartmentEditDialog dialog(this, dept);

    if (dialog.exec() == QDialog::Accepted) {
        Department updatedDept = dialog.getDepartment();

        DepartmentRepository repo;
        if (repo.update(updatedDept)) {
            QMessageBox::information(this, "成功", "部门更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "部门更新失败！");
        }
    }
}

void DepartmentManagementPage::onDeleteClicked()
{
    Department dept = getSelectedDepartment();
    if (dept.id <= 0) {
        QMessageBox::warning(this, "提示", "请先选择要删除的部门！");
        return;
    }

    DepartmentRepository repo;

    // Check if department has children
    auto allDepts = repo.find_all();
    for (const auto& d : allDepts) {
        if (d->parent_department_id == dept.id) {
            QMessageBox::warning(this, "提示", "该部门下还存在下级部门，不能直接删除！");
            return;
        }
    }

    // Check if department has users
    UserRepository userRepo;
    auto users = userRepo.findByDepartmentId(dept.id);
    if (!users.isEmpty()) {
        QMessageBox::warning(this, "提示", "该部门下还存在用户，不能直接删除！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除部门 '%1' 吗？").arg(QString::fromStdString(dept.name)),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (repo.remove(dept.id)) {
            QMessageBox::information(this, "成功", "部门已删除！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "部门删除失败！");
        }
    }
}

void DepartmentManagementPage::onRefreshClicked()
{
    loadData();
}

void DepartmentManagementPage::onTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);

    if (!item) {
        return;
    }

    int deptId = item->data(0, Qt::UserRole).toInt();

    DepartmentRepository repo;
    auto dept = repo.find_by_id(deptId);

    if (!dept) {
        return;
    }

    DepartmentEditDialog dialog(this, *dept);

    if (dialog.exec() == QDialog::Accepted) {
        Department updatedDept = dialog.getDepartment();

        if (repo.update(updatedDept)) {
            QMessageBox::information(this, "成功", "部门更新成功！");
            loadData();
        } else {
            QMessageBox::critical(this, "错误", "部门更新失败！");
        }
    }
}
