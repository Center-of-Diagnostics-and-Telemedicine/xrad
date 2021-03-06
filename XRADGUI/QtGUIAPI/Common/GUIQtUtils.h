﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file GUIQtUtils.h
//--------------------------------------------------------------
#ifndef XRAD__File_GUIQtUtils_h
#define XRAD__File_GUIQtUtils_h
//--------------------------------------------------------------

#include <type_traits>

namespace XRAD_GUI
{

//! \brief Сокращение для std::remove_pointer_t, используется в вызовах connect
template <class T>
using RP = std::remove_pointer_t<T>;

//! \brief Обертка для QObject::connect, позволяющая в именах методов сигнала и слота
//! не писать имена типов sender и receiver
#define XRAD_GUI_connect_auto(sender, sender_signal, receiver, receiver_slot) \
	QObject::connect( \
		sender, &RP<decltype(sender)>::sender_signal, \
		receiver, &RP<decltype(receiver)>::receiver_slot)

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_GUIQtUtils_h
