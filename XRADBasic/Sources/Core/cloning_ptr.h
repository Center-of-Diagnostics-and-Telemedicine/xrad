/*!
	* \file cloning_ptr.h
	* \date 12/26/2018 1:21:44 PM
	*
	* \author Kovbas (kovbas)
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef cloning_ptr_h__
#define cloning_ptr_h__

#include "Config.h"
#include "BasicMacros.h"

#include <memory> //for unique_ptr

XRAD_BEGIN

//Есть идея создать еще один вариант умного указателя для полиморфных классов.
//Унаследовать его от unique_ptr и добавить копирующий конструктор и оператор = .
//При этом копирование будет создавать копию объекта, находящегося по указателю.
//У объектов, которые передаются такому указателю, должен быть виртуальный метод self *clone(), который создает копию объекта именно того типа, который там хранится, а не родителя
//Мы часто используем указатели, чтобы можно было хранить список абстрактных источников данных, все подробности реализации которых хранятся в их наследниках.
//Это приводит к тому, что при копировании списков копируются указатели(shared_ptr).
//В результате если мы поменяли что - либо в одном из списков, все другие списки, работающие с этим источником, также будут затронуты.
//Не было бы проблемы, если бы мы хранили сами объекты : копии создавались бы автоматически.
//Собственно, предполагается, что это решит проблему

//TODO нужно продокументировать материал, убрать неиспользуемый код, эксперименты и т.п. Примеры поместить в соответствующие секции комментариев, а не в компилируемый код
/*!
	\brief Класс реализует умный указатель unique_ptr к объекту с возможностью копирования. При копировании создаётся новый указатель на копию объекта, на которую указывал изначальный указатель. Копирование объекта полиморфно с учётом описанных ниже требований.
	\details Для использования cloning_ptr требуется соблюдение следующих условий:
	1. в управляемых объектах должны быть функции clone, создающие в памяти копии объектов, на которые указывает unique_ptr, и возвращающие указатель на созданный ими объект.
	2. наличие виртуального деструктора в базовом классе для исключения утечек памяти
	3. Важное отличие от общего unique_ptr -- использование только deleter'а по умолчанию. Базовый класс допускал использование функций (например, free). Здесь разрешено только Ty::~Ty().

	// Примеры использования:
	cloning_ptr<classA> classA_ptr;
	classA_ptr = (new classA());
	classA_ptr = make_unique<classA>();
	cloning_ptr<classA> classA_ptr1(classA_ptr);
	cloning_ptr<classA> classA_ptr2;
	classA_ptr2 = classA_ptr1;

	// Пример иерархии классов, в которых выполнены указанные выше требования:
	// class examples:
	class classA
	{
	public:
		classA() {}
		virtual ~classA() {}
		virtual void print()
		{
			printf("I'm classA\n");
		}
		virtual classA* clone() const
		{
			return (new classA(*this));
		}
	private:
		int a = 4;
	};

	class classB : public classA
	{
	public:
		classB() {}
		virtual ~classB() {}
		virtual void print() override
		{
			printf("I'm classB\n");
		}
		virtual classB* clone() const override
		{
			return (new classB(*this));
		}
	private:
		int b = 6;
	};

	class classC : public classB
	{
	public:
		classC() {}
		virtual ~classC() {}
		virtual void print() override
		{
			printf("I'm classC\n");
		}
		virtual classC* clone() const override
		{
			return (new classC(*this));
		}
	private:
		int c = 8;
	};
*/
template <class Ty>
class cloning_ptr : public unique_ptr<Ty>
{
	typedef cloning_ptr<Ty> self;
	using parent = unique_ptr<Ty>;
	void	clone_util(const self &other) { reset(other ? other->clone() : nullptr); }

public:
	using parent::reset;

public:
	cloning_ptr() {}

	cloning_ptr(const self& other) { clone_util(other); }
	cloning_ptr(self&& other) { reset(other.release()); }
	cloning_ptr(Ty *other) { reset(other); }
	cloning_ptr(const unique_ptr<Ty> &other) { clone_util(other); }
	cloning_ptr(unique_ptr<Ty> && other) { reset(other.release()); }

	self& operator=(const self& other) { clone_util(other); return *this; }
	self& operator=(self&& other){ reset(other.release()); return *this; }
	self& operator=(Ty* other) { reset(other); return *this; }
	self& operator=(const unique_ptr<Ty> &other) { clone_util(other); return *this; }
	self& operator=(unique_ptr<Ty> &&other) {reset(other.release()); return *this;}
};

/*
template <class Ty>
cloning_ptr<Ty> make_cloning(Ty&obj)
{

}*/


XRAD_END

#endif // cloning_ptr_h__