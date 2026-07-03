/****************************************************************************
** Meta object code from reading C++ file 'ExpressionBuilder.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../../source/applications/gui/qt/GenesysQtGUI/tools/expressionbuilder/ExpressionBuilder.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ExpressionBuilder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_ExpressionBuilder_t {
    uint offsetsAndSizes[20];
    char stringdata0[18];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[17];
    char stringdata4[5];
    char stringdata5[7];
    char stringdata6[22];
    char stringdata7[17];
    char stringdata8[27];
    char stringdata9[17];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ExpressionBuilder_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_ExpressionBuilder_t qt_meta_stringdata_ExpressionBuilder = {
    {
        QT_MOC_LITERAL(0, 17),  // "ExpressionBuilder"
        QT_MOC_LITERAL(18, 19),  // "on_categorySelected"
        QT_MOC_LITERAL(38, 0),  // ""
        QT_MOC_LITERAL(39, 16),  // "QTreeWidgetItem*"
        QT_MOC_LITERAL(56, 4),  // "item"
        QT_MOC_LITERAL(61, 6),  // "column"
        QT_MOC_LITERAL(68, 21),  // "on_expressionSelected"
        QT_MOC_LITERAL(90, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(107, 26),  // "on_copyToClipboard_clicked"
        QT_MOC_LITERAL(134, 16)   // "on_close_clicked"
    },
    "ExpressionBuilder",
    "on_categorySelected",
    "",
    "QTreeWidgetItem*",
    "item",
    "column",
    "on_expressionSelected",
    "QListWidgetItem*",
    "on_copyToClipboard_clicked",
    "on_close_clicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ExpressionBuilder[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   38,    2, 0x08,    1 /* Private */,
       6,    1,   43,    2, 0x08,    4 /* Private */,
       8,    0,   46,    2, 0x08,    6 /* Private */,
       9,    0,   47,    2, 0x08,    7 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 7,    4,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject ExpressionBuilder::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ExpressionBuilder.offsetsAndSizes,
    qt_meta_data_ExpressionBuilder,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ExpressionBuilder_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ExpressionBuilder, std::true_type>,
        // method 'on_categorySelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QTreeWidgetItem *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_expressionSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'on_copyToClipboard_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_close_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void ExpressionBuilder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ExpressionBuilder *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_categorySelected((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->on_expressionSelected((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 2: _t->on_copyToClipboard_clicked(); break;
        case 3: _t->on_close_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject *ExpressionBuilder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExpressionBuilder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ExpressionBuilder.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ExpressionBuilder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
