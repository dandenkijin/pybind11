// Copyright (c) 2024 The pybind Community.

// In production situations it is totally fine to build with
// C++ Exception Handling enabled. However, here we want to ensure that
// C++ Exception Handling is not required.
#if defined(_MSC_VER) || defined(__EMSCRIPTEN__)
// Too much trouble making the required cmake changes (see PR #5375).
#else
#    ifdef __cpp_exceptions
#    error This test is meant to be built with C++ Exception Handling disabled, but __cpp_exceptions is defined.
#    endif
#    ifdef __EXCEPTIONS
#    error This test is meant to be built with C++ Exception Handling disabled, but __EXCEPTIONS is defined.
#    endif
#endif

// THIS MUST STAY AT THE TOP!
#include <pybind11/conduit/pybind11_conduit_v1.h> // VERY light-weight dependency.

#include "test_cpp_conduit_traveler_types.h"

#include <Python.h>
#include <typeinfo>

namespace {

void *get_cpp_conduit_void_ptr(PyObject *py_obj, const std::type_info *cpp_type_info) {
    PyObject *cpp_type_info_capsule
        = PyCapsule_New(const_cast<void *>(static_cast<const void *>(cpp_type_info)),
                        typeid(std::type_info).name(),
                        nullptr);
    if (cpp_type_info_capsule == nullptr) {
        return nullptr;
    }
    PyObject *cpp_conduit = PyObject_CallMethod(py_obj,
                                                "_pybind11_conduit_v1_",
                                                "yOy",
                                                PYBIND11_PLATFORM_ABI_ID,
                                                cpp_type_info_capsule,
                                                "raw_pointer_ephemeral");
    Py_DECREF(cpp_type_info_capsule);
    if (cpp_conduit == nullptr) {
        return nullptr;
    }
    void *void_ptr = PyCapsule_GetPointer(cpp_conduit, cpp_type_info->name());
    Py_DECREF(cpp_conduit);
    if (PyErr_Occurred()) {
        return nullptr;
    }
    return void_ptr;
}

template <typename T>
T *get_cpp_conduit_type_ptr(PyObject *py_obj) {
    void *void_ptr = get_cpp_conduit_void_ptr(py_obj, &typeid(T));
    if (void_ptr == nullptr) {
        return nullptr;
    }
    return static_cast<T *>(void_ptr);
}

extern "C" PyObject *wrapGetLuggage(PyObject * /*self*/, PyObject *traveler) {
    const auto *cpp_traveler
        = get_cpp_conduit_type_ptr<pybind11_tests::test_cpp_conduit::Traveler>(traveler);
    if (cpp_traveler == nullptr) {
        return nullptr;
    }
    return PyUnicode_FromString(cpp_traveler->luggage.c_str());
}

extern "C" PyObject *wrapGetPoints(PyObject * /*self*/, PyObject *premium_traveler) {
    const auto *cpp_premium_traveler
        = get_cpp_conduit_type_ptr<pybind11_tests::test_cpp_conduit::PremiumTraveler>(
            premium_traveler);
    if (cpp_premium_traveler == nullptr) {
        return nullptr;
    }
    return PyLong_FromLong(static_cast<long>(cpp_premium_traveler->points));
}

PyMethodDef ThisMethodDef[] = {{"GetLuggage", wrapGetLuggage, METH_O, nullptr},
                               {"GetPoints", wrapGetPoints, METH_O, nullptr},
                               {nullptr, nullptr, 0, nullptr}};

struct PyModuleDef ThisModuleDef = {
    PyModuleDef_HEAD_INIT, // m_base
    "exo_planet_c_api",    // m_name
    nullptr,               // m_doc
    -1,                    // m_size
    ThisMethodDef,         // m_methods
    nullptr,               // m_slots
    nullptr,               // m_traverse
    nullptr,               // m_clear
    nullptr                // m_free
};

} // namespace

#if defined(WIN32) || defined(_WIN32)
#    define EXO_PLANET_C_API_EXPORT __declspec(dllexport)
#else
#    define EXO_PLANET_C_API_EXPORT __attribute__((visibility("default")))
#endif

extern "C" EXO_PLANET_C_API_EXPORT PyObject *PyInit_exo_planet_c_api() {
    PyObject *m = PyModule_Create(&ThisModuleDef);
    if (m == nullptr) {
        return nullptr;
    }
    return m;
}
