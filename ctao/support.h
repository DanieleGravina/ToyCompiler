#ifndef SUPPORT_H
#define	SUPPORT_H

#ifdef _WIN32
#define typeof typeid
#endif

#define foreach(IT,X) for( typeof( X->begin() ) IT = X->begin(); IT != X->end(); ++IT )

#endif