#include "familymember.h"
#include "ui_familymember.h"
#include <QRegExpValidator>
#include <QSqlError>
#include <QDebug>

FamilyMember::FamilyMember(int ownerId, DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::FamilyMember),
      memberId(-1),
      ownerId(ownerId),
      dbManager(dbManager) // 使用传入的数据库管理器实例
{
    ui->setupUi(this);

    // 设置验证器（修正身份证号验证，仅允许数字，匹配表结构INTEGER类型）
    ui->phoneEdit->setValidator(new QRegExpValidator(
        QRegExp("^1[0-9]{10}$"), this)); // 11位手机号
    ui->idNumberEdit->setValidator(new QRegExpValidator(
        QRegExp("^[0-9]{18}$"), this)); // 18位数字（表结构为INTEGER，不允许X）

    // 信号连接
    connect(ui->okBtn, &QPushButton::clicked, this, &FamilyMember::accept);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    // 检查数据库连接状态
    if (!dbManager->isOpen()) {
        qWarning() << "传入的数据库连接未打开：" << dbManager->lastError();
    }
}

FamilyMember::~FamilyMember()
{
    delete ui;
}

void FamilyMember::setMemberData(int memberId, const QString &name, const QString &relation,
                                 const QString &phone, const QString &idNumber)
{
    this->memberId = memberId;
    ui->nameEdit->setText(name);
    ui->relationCombo->setCurrentText(relation);
    ui->phoneEdit->setText(phone);
    ui->idNumberEdit->setText(idNumber);
}

void FamilyMember::accept()
{
    if (!dbManager->isOpen()) {
        qWarning() << "数据库未打开，无法执行操作";
        return;
    }

    // 从UI控件读取数据
    QString name = ui->nameEdit->text().trimmed();
    QString relation = ui->relationCombo->currentText();
    QString phoneStr = ui->phoneEdit->text().trimmed();
    QString idNumberStr = ui->idNumberEdit->text().trimmed();

    // 输入校验
    if (name.isEmpty() || phoneStr.isEmpty() || idNumberStr.isEmpty()) {
        qWarning() << "输入内容不完整，请检查";
        return;
    }

    // 转换手机号为整数（匹配表结构INTEGER类型）
    bool phoneOk;
    qlonglong phone = phoneStr.toLongLong(&phoneOk);
    if (!phoneOk) {
        qWarning() << "手机号格式错误，无法转换为数字";
        return;
    }

    // 转换身份证号为整数（匹配表结构INTEGER类型）
    bool idOk;
    qlonglong idNumber = idNumberStr.toLongLong(&idOk);
    if (!idOk) {
        qWarning() << "身份证号格式错误，无法转换为数字";
        return;
    }

    // 开启事务
    if (!dbManager->beginTransaction()) {
        qWarning() << "开启事务失败：" << dbManager->lastError();
        return;
    }

    bool success = false;
    if (memberId == -1) {
        // 新增数据（字段顺序与表结构对应）
        QString sql = "INSERT INTO family_member (owner_id, name, relationship, phone, id_number) VALUES (?, ?, ?, ?, ?)";
        QVariantList bindValues = {ownerId, name, relation, phone, idNumber};
        success = dbManager->executeQuery(sql, bindValues);
    } else {
        // 更新数据（WHERE条件匹配主键和关联字段）
        QString sql = "UPDATE family_member SET name = ?, relationship = ?, phone = ?, id_number = ? WHERE member_id = ? AND owner_id = ?";
        QVariantList bindValues = {name, relation, phone, idNumber, memberId, ownerId};
        success = dbManager->executeQuery(sql, bindValues);
    }

    if (success) {
        if (dbManager->commitTransaction()) {
            QDialog::accept(); // 关闭对话框，返回 Accepted
        } else {
            qWarning() << "提交事务失败：" << dbManager->lastError();
            dbManager->rollbackTransaction();
        }
    } else {
        qWarning() << "执行SQL失败：" << dbManager->lastError();
        dbManager->rollbackTransaction();
    }
}
