/****************************************************************************
** Meta object code from reading C++ file 'SettingsDialog.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../app/ui/SettingsDialog.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SettingsDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14SettingsDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto SettingsDialog::qt_create_metaobjectdata<qt_meta_tag_ZN14SettingsDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SettingsDialog",
        "hotkeyChanged",
        "",
        "QKeySequence",
        "autoPasteChanged",
        "pasteDelayChanged",
        "preloadChanged",
        "cleanupRequested",
        "days",
        "usageSkipGreaterThan",
        "windowSizeChanged",
        "useCurrentWindowSizeRequested"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'hotkeyChanged'
        QtMocHelpers::SignalData<void(QKeySequence)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 2 },
        }}),
        // Signal 'autoPasteChanged'
        QtMocHelpers::SignalData<void(bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 2 },
        }}),
        // Signal 'pasteDelayChanged'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'preloadChanged'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 2 },
        }}),
        // Signal 'cleanupRequested'
        QtMocHelpers::SignalData<void(int, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 },
        }}),
        // Signal 'windowSizeChanged'
        QtMocHelpers::SignalData<void(QSize)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QSize, 2 },
        }}),
        // Signal 'useCurrentWindowSizeRequested'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SettingsDialog, qt_meta_tag_ZN14SettingsDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SettingsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14SettingsDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14SettingsDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14SettingsDialogE_t>.metaTypes,
    nullptr
} };

void SettingsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SettingsDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->hotkeyChanged((*reinterpret_cast< std::add_pointer_t<QKeySequence>>(_a[1]))); break;
        case 1: _t->autoPasteChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->pasteDelayChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->preloadChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->cleanupRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->windowSizeChanged((*reinterpret_cast< std::add_pointer_t<QSize>>(_a[1]))); break;
        case 6: _t->useCurrentWindowSizeRequested(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)(QKeySequence )>(_a, &SettingsDialog::hotkeyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)(bool )>(_a, &SettingsDialog::autoPasteChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)(int )>(_a, &SettingsDialog::pasteDelayChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)(int )>(_a, &SettingsDialog::preloadChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)(int , int )>(_a, &SettingsDialog::cleanupRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)(QSize )>(_a, &SettingsDialog::windowSizeChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SettingsDialog::*)()>(_a, &SettingsDialog::useCurrentWindowSizeRequested, 6))
            return;
    }
}

const QMetaObject *SettingsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14SettingsDialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int SettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void SettingsDialog::hotkeyChanged(QKeySequence _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void SettingsDialog::autoPasteChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void SettingsDialog::pasteDelayChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void SettingsDialog::preloadChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void SettingsDialog::cleanupRequested(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void SettingsDialog::windowSizeChanged(QSize _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void SettingsDialog::useCurrentWindowSizeRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
