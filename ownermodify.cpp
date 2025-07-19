#include "ownermodify.h"
#include "ui_ownermodify.h"
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDateTime>

OwnerModify::OwnerModify(QWidget *parent)
    : QWidget(parent)
   , ui(new Ui::OwnerModify)
   , dbManager(new DatabaseManager(this))
{
    QString connName = QString("OwnerModify_%1")
               .arg(QDateTime::currentMSecsSinceEpoch());

    ui->setupUi(this);
    initUI();

    // 初始化数据库并加载数据
    if (!dbManager->initDatabase(connName)) {
        QMessageBox::critical(this, "数据库错误", "初始化数据库失败：" + dbManager->lastError());
        return;
    }
    loadAllOwners();
}

OwnerModify::~OwnerModify()
{
    delete ui;
    // dbManager由父类析构，自动关闭数据库
}

void OwnerModify::initUI()
{
    // 设置表格属性
    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setHorizontalHeaderLabels({
        "业主ID", "姓名", "身份证号", "电话", "邮箱", "入住日期", "注册时间"
    });
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 表格不可直接编辑
    updateButtonState(false); // 初始禁用修改/删除按钮
}

bool OwnerModify::loadAllOwners()
{
    // 调用数据库管理类查询所有业主（使用owner_info表结构）
    QVector<QVariantMap> owners = dbManager->executeSelectQuery("SELECT * FROM owner_info");
    if (owners.isEmpty() &&!dbManager->lastError().isEmpty()) {
        QMessageBox::warning(this, "查询警告", "加载业主信息失败：" + dbManager->lastError());
        return false;
    }
    showOwners(owners);
    return true;
}

void OwnerModify::showOwners(const QVector<QVariantMap>& owners)
{
    ui->tableWidget->setRowCount(0); // 清空表格
    for (const auto& owner : owners) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        // 按owner_info表字段顺序填充（与表格列对应）
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(owner["owner_id"].toString()));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(owner["name"].toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(owner["id_number"].toString()));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(owner["phone_number"].toString()));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(owner["email"].toString()));
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(owner["move_in_date"].toString()));
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(owner["register_time"].toString()));
    }
}

void OwnerModify::updateButtonState(bool hasSelection)
{
    ui->modifyButton->setEnabled(hasSelection);
    ui->deleteButton->setEnabled(hasSelection);
}

void OwnerModify::on_tableWidget_itemSelectionChanged()
{
    // 检查是否有选中项
    bool hasSelection =!ui->tableWidget->selectedItems().isEmpty();
    updateButtonState(hasSelection);
}

void OwnerModify::on_refreshButton_clicked()
{
    loadAllOwners(); // 刷新数据
}

void OwnerModify::on_modifyButton_clicked()
{
    // 获取选中行
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    if (selectedItems.isEmpty()) return;
    int row = selectedItems.first()->row();

    // 获取业主ID（不可修改）
    QString ownerId = ui->tableWidget->item(row, 0)->text();
    if (ownerId.isEmpty()) {
        QMessageBox::warning(this, "错误", "无效的业主ID");
        return;
    }

    // 获取当前数据
    QString currentName = ui->tableWidget->item(row, 1)->text();
    QString currentIdNum = ui->tableWidget->item(row, 2)->text();
    QString currentPhone = ui->tableWidget->item(row, 3)->text();
    QString currentEmail = ui->tableWidget->item(row, 4)->text();
    QString currentMoveIn = ui->tableWidget->item(row, 5)->text();

    // 创建修改对话框
    QDialog dialog(this);
    dialog.setWindowTitle("修改业主信息");
    dialog.setMinimumWidth(400);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // 输入框（预填当前值）
    QLineEdit *nameEdit = new QLineEdit(currentName, &dialog);
    QLineEdit *idNumEdit = new QLineEdit(currentIdNum, &dialog);
    QLineEdit *phoneEdit = new QLineEdit(currentPhone, &dialog);
    QLineEdit *emailEdit = new QLineEdit(currentEmail, &dialog);
    QLineEdit *moveInEdit = new QLineEdit(currentMoveIn, &dialog);

    // 添加到布局
    layout->addWidget(new QLabel("姓名:", &dialog));
    layout->addWidget(nameEdit);
    layout->addWidget(new QLabel("身份证号:", &dialog));
    layout->addWidget(idNumEdit);
    layout->addWidget(new QLabel("电话:", &dialog));
    layout->addWidget(phoneEdit);
    layout->addWidget(new QLabel("邮箱:", &dialog));
    layout->addWidget(emailEdit);
    layout->addWidget(new QLabel("入住日期:", &dialog));
    layout->addWidget(moveInEdit);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *saveBtn = new QPushButton("保存", &dialog);
    QPushButton *cancelBtn = new QPushButton("取消", &dialog);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // 连接按钮信号
    connect(saveBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    // 显示对话框并处理结果
    if (dialog.exec() == QDialog::Accepted) {
        // 准备更新语句（匹配owner_info表字段）
        QString sql = "UPDATE owner_info SET "
                      "name =?, "
                      "id_number =?, "
                      "phone_number =?, "
                      "email =?, "
                      "move_in_date =? "
                      "WHERE owner_id =?";

        // 绑定参数（按顺序）
        QVariantList params;
        params << nameEdit->text()
               << idNumEdit->text()
               << phoneEdit->text()
               << emailEdit->text()
               << moveInEdit->text()
               << ownerId;

        // 执行更新（使用数据库管理类）
        if (dbManager->executeQuery(sql, params)) {
            QMessageBox::information(this, "成功", "业主信息修改成功");
            loadAllOwners(); // 刷新表格
        } else {
            QMessageBox::critical(this, "失败", "修改失败：" + dbManager->lastError());
        }
    }
}

void OwnerModify::on_deleteButton_clicked()
{
    // 获取选中行
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
    if (selectedItems.isEmpty()) return;
    int row = selectedItems.first()->row();

    // 获取业主信息
    QString ownerId = ui->tableWidget->item(row, 0)->text();
    QString name = ui->tableWidget->item(row, 1)->text();
    if (ownerId.isEmpty()) {
        QMessageBox::warning(this, "错误", "无效的业主ID");
        return;
    }

    // 确认删除
    if (QMessageBox::question(this, "确认", "确定要删除业主【" + name + "】吗？")!= QMessageBox::Yes) {
        return;
    }

    // 执行删除（使用数据库管理类）
    QString sql = "DELETE FROM owner_info WHERE owner_id =?";
    QVariantList params;
    params << ownerId;

    if (dbManager->executeQuery(sql, params)) {
        QMessageBox::information(this, "成功", "业主信息已删除");
        loadAllOwners(); // 刷新表格
    } else {
        QMessageBox::critical(this, "失败", "删除失败：" + dbManager->lastError());
    }
}
