#ifndef LRCABSTRACTLISTMODELBASE_H
#define LRCABSTRACTLISTMODELBASE_H
#pragma once

#include <QAbstractListModel>

class LRCAbstractListModelBase : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit LRCAbstractListModelBase(QObject* parent = nullptr)
        : QAbstractListModel(parent) {}
    ~LRCAbstractListModelBase() = default;
};

#endif // LRCABSTRACTLISTMODELBASE_H
