#include "databasemanager.h"
#include <QDir>
#include <QSqlRecord>
#include <QSqlDriver>

//构造函数初始化成员变量
DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent), m_isOpen(false) {
    // 空实现，初始化逻辑移到initDatabase()
}

//析构函数关闭数据库
DatabaseManager::~DatabaseManager() {
    if (m_database.isOpen()) {
        m_database.close();
    }
}

//初始化数据库
bool DatabaseManager::initDatabase(const QString &connectionName) {
    QString dbPath = QDir::currentPath() + "/database.db";
    QDir dbDir = QDir::current();
    if (!dbDir.exists(QFileInfo(dbPath).absolutePath())) {
        if (!dbDir.mkpath(QFileInfo(dbPath).absolutePath())) {
            return false;
        }
    }
    //addDatabase的第二个参数为连接名（避免默认连接冲突）
    m_database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    m_database.setDatabaseName(dbPath);
    if (!m_database.open()) {
        return false;
    }
    m_isOpen = true;
    emit databaseOpened(true, "数据库打开成功");
    return true;
}

bool DatabaseManager::isOpen() const {
    return m_isOpen && m_database.isOpen();
}

QString DatabaseManager::lastError() const {
    return m_database.lastError().text();
}

bool DatabaseManager::executeQuery(const QString& queryString, const QVariantList& bindValues) {
    if (!isOpen()) {
        emit queryExecuted(false, "数据库未打开");
        return false;
    }
    QSqlQuery query(m_database);  // 使用当前实例的数据库连接
    query.prepare(queryString);
    for (int i = 0; i < bindValues.size(); ++i) {
        query.bindValue(i, bindValues[i]);
    }
    if (!query.exec()) {
        QString errorMsg = QString("查询执行失败: %1 (SQL: %2)").arg(query.lastError().text(), queryString);
        emit queryExecuted(false, errorMsg);
        return false;
    }
    emit queryExecuted(true, "查询执行成功");
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        // 获取当前连接名称
        QString connectionName = m_database.connectionName();

        // 关闭数据库连接
        m_database.close();

        // 从QSqlDatabase中移除连接
        // 注意：必须在数据库关闭后调用
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QVector<QVariantMap> DatabaseManager::executeSelectQuery(const QString& queryString, const QVariantList& bindValues)
{
    QVector<QVariantMap> resultList;

    // 增强数据库连接检查
    if (!isOpen()) {
        QString errorMsg = "数据库未打开";
        emit queryExecuted(false, errorMsg);
        return resultList;
    }

    QSqlQuery query(m_database);
    query.prepare(queryString);

    for (int i = 0; i < bindValues.size(); ++i) {
        query.bindValue(i, bindValues[i]);
    }

    // 执行查询并处理结果
    if (!query.exec()) {
        QString errorMsg = QString("查询执行失败: %1 (SQL: %2)")
                           .arg(query.lastError().text(), queryString);
        emit queryExecuted(false, errorMsg);
        return resultList;
    }

    // 处理查询结果
    QSqlRecord record = query.record();

    while (query.next()) {
        QVariantMap rowData;
        for (int i = 0; i < record.count(); ++i) {
            rowData[record.fieldName(i)] = query.value(i);
        }
        resultList.append(rowData);
    }

    emit queryExecuted(true, QString("查询成功，返回 %1 条记录").arg(resultList.size()));

    return resultList;
}

bool DatabaseManager::beginTransaction() {
    if (!isOpen()) return false;
    return m_database.transaction();
}

bool DatabaseManager::commitTransaction() {
    if (!isOpen()) return false;
    return m_database.commit();
}

bool DatabaseManager::rollbackTransaction() {
    if (!isOpen()) return false;
    return m_database.rollback();
}
