#ifndef DATASET_H
#define DATASET_H

#include <vector>

namespace WorldGen
{
	class Database;

	template < class T >
	class DataSet
	{
	public:
		DataSet( Database* parent )
		{
			Parent_ = parent;
		}

		T* Create()
		{
			int newId = NextId;
			++NextId;
			return Create( newId );
		}

		T* Create( int NewId )
		{
			if ( GetItemById( NewId ) == nullptr )
				Data.push_back( new T( NewId ) );
			GetItemById( NewId )->Database_ = Parent_;
			return GetItemById( NewId );
		}

		void Remove( int Id )
		{
			for ( int Idx = 0; Idx < Data.size(); ++Idx )
			{
				if ( Data[ Idx ]->Id_ == Id )
				{
					Data.erase( Data.begin() + Idx );
					return;
				}
			}
		}

		T* GetItemById( int Id )
		{
			for ( unsigned int Idx = 0; Idx < Data.size(); ++Idx )
			{
				if ( Data[ Idx ]->Id_ == Id )
				{
					return Data[ Idx ];
				}
			}
			return nullptr;
		}

		T* GetItem( int Pos )
		{
			return Data[ Pos ];
		}

		size_t Size()
		{
			return Data.size();
		}

		std::vector<T*>& Internal()
		{
			return Data;
		}
	private:
		std::vector< T* > Data;
		int NextId = 0;
		Database* Parent_;
	};

#define MAP_ONE( ObjectName, TableName, TypeName, IdName ) TypeName* Get ## ObjectName() \
	{ \
		return Database_-> ## TableName ## .GetItemById(IdName);\
	} \
	\
	void Set ## ObjectName (TypeName * Obj) \
	{ \
		Data* d = (Data*)Obj;\
		IdName = d->Id_;\
	}

#define MAP_MANY( ObjectName, TableName, TypeName, IdVector, OwnerType ) \
	class AUTODBOBJECTMAPPER_ ## ObjectName ## TypeName { \
		OwnerType* ObjectOwner_; \
	public: \
		class Iterator; \
		AUTODBOBJECTMAPPER_ ## ObjectName ## TypeName () { } \
		AUTODBOBJECTMAPPER_ ## ObjectName ## TypeName ( OwnerType* owner ) \
			: ObjectOwner_( owner ) \
		{} \
		std::vector< TypeName* > Get ## ObjectName() \
		{ \
			std::vector< TypeName* > ret; \
			for (int Idx = 0; Idx < (int)ObjectOwner_->IdVector.size(); ++Idx) \
				ret.push_back( ObjectOwner_->Database_-> ## TableName ## .GetItemById( ObjectOwner_->IdVector[ Idx ] )); \
			return ret;\
		} \
		int Size() \
		{ \
			return (int)ObjectOwner_->IdVector.size(); \
		} \
		Iterator Begin(){return Iterator(ObjectOwner_, 0);}\
		Iterator End(){return Iterator(ObjectOwner_, Size());}\
		void Remove( TypeName* Obj)  \
		{ \
			std::vector<int>::iterator iter; \
			for (iter = ObjectOwner_->IdVector.begin(); iter != ObjectOwner_->IdVector.end();)\
			{\
				Data* d = (Data*)Obj;\
				if ((*iter) == d->Id_) \
					iter = ObjectOwner_->IdVector.erase(iter);\
				else\
					iter++;\
			}\
		}; \
		void Add( TypeName* Obj ) \
		{ \
			Data* d = (Data*)Obj; ObjectOwner_->IdVector.push_back(d->Id_); \
		} \
		TypeName* operator[] (int Idx ) \
		{ \
			return ObjectOwner_->Database_-> ## TableName ## .GetItemById( ObjectOwner_->IdVector[ Idx ] ) ; \
		}\
		class Iterator {  \
		private: \
			OwnerType* ObjectOwner_; \
			int Index; \
		public: \
			Iterator(OwnerType* owner) : ObjectOwner_ (owner) {}\
			Iterator(OwnerType* owner, int idx) : ObjectOwner_ (owner), Index(idx) {}\
			TypeName* operator* ( ) \
			{ \
				return ObjectOwner_->Database_-> ## TableName ## .GetItemById( ObjectOwner_->IdVector[ Index ] ); \
			} \
			Iterator operator++() \
			{\
				Iterator itr(ObjectOwner_); itr.Index = Index + 1; return itr; \
			}\
			bool operator==(Iterator & oth) { return oth.Index == Index ; } \
				bool operator!=(Iterator & oth) { return oth.Index != Index ; } \
			}; \
		} ;\
		AUTODBOBJECTMAPPER_ ## ObjectName ## TypeName ObjectName = AUTODBOBJECTMAPPER_ ## ObjectName ## TypeName (this);

}
#endif