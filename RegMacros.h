#define SIZEOFARRAY(x)  (sizeof(x) / sizeof(x[0]))

#define MTYPEV(guid) {&MEDIATYPE_Video, &guid}
#define MTYPES(guid) {&MEDIATYPE_Stream, &guid}

#define DEFINEPIN(nam, out, type) { L#nam, FALSE, out, FALSE, FALSE, &CLSID_NULL, NULL, SIZEOFARRAY(type), type }

#define DEFINEFILTER(guid, name, merit, info) { &guid, name, merit, SIZEOFARRAY(info), info }

#define DEFINETEMPLATE(name, guid, create, filt) { name, &guid, create, NULL, filt }
