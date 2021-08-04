/*
	xml_valid(xml)
	Returns 1 if the argument is well-formed XML and 0, otherwise.
	select xml_valid('<a>A</a>'); --> 1
	select xml_valid('<a>A/a>'); --> 0
	
	xml_extract(xml, xpath, sep = "")
	Extracts a node content or an attribute value.
	select xml_extract('<a>A</a>', 'a'); --> <a>A</a>
	select xml_extract('<a>A</a>', 'a/text()'); --> A
	select xml_extract('<a>A</a><a>B</a>', 'a/text()', ','); --> A,B	
	select xml_extract('<a id = "1">A</a>', 'a/@id'); --> 1
	
	xml_append(xml, xpath, insertion, pos = after)
	Appends a node or an attribute based on the pos (one of: first, before, after, last, child or "child first"/"child last"). 
	The "child" is ignored for the attribute. The insertion should be valid (there is no check).
	select xml_append('<a>A</a><a>B</a><a>C</a>', 'a[2]', '<b>D</b>', 'after') xml; 
	--> <a>A</a><a>B</a><b>D</b><a>C</a>
	select xml_append('<a>A</a><a>B</a><a>C</a>', 'a[2]', '<b>D</b>', 'child') xml; 
	--> <a>A</a><a>B<b>D</b></a><a>C</a>
	select xml_append('<a>A</a><a id="1">B</a><a id="2">C</a>', 'a/@id', 'x="2"', 'first') xml; 
	--> <a>A</a><a x="2" id="1">B</a><a x="2" id="2">C</a>
	
	xml_update(xml, xpath, replacement)
	Updates nodes or attributes. The replacement should be valid (there is no check). 
	If the replacement is NULL then the call equals to xml_remove(xml, path).
	select xml_update('<a>A</a><a id="1">B</a><a id="2">C</a>', 'a[2]', '<b>D</b>');
	--> <a>A</a><b>D</b><a id="2">C</a>
	select xml_update('<a>A</a><a id="1">B</a><a id="2">C</a>', 'a/@id', '3');
	--> <a>A</a><a id="3">B</a><a id="3">C</a>
	
	xml_remove(xml, xpath)
	Removes nodes or attributes.
	select xml_remove('<a>A</a><a id="1">B</a><a id="2">C</a>', 'a[2]');
	--> <a>A</a><a id="2">C</a>
	select xml_remove('<a>A</a><a id="1">B</a><a id="2">C</a>', 'a/@id');
	--> <a>A</a><a>B</a><a>C</a>
	
	xml_each(xml, xpath)
	It's a table-valued function.
	select * from xml_each('<a>A</a><a>B</a><a>C</a>', 'a/text()'); --> Three rows: A, B and C
*/

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "pugixml.hpp"
extern "C" {
	#include "uthash.h"
}

/* Auxiliaries */
struct xml_memory_writer: pugi::xml_writer {
	char* buffer;
	size_t capacity;
	size_t result;

	xml_memory_writer(): buffer(0), capacity(0), result(0) { }
	xml_memory_writer(char* buffer, size_t capacity): buffer(buffer), capacity(capacity), result(0) { }
	size_t written_size() const {
		return result < capacity ? result : capacity;
	}

	virtual void write(const void* data, size_t size) {
		if (result < capacity) {
			size_t chunk = (capacity - result < size) ? capacity - result : size;
			memcpy(buffer + result, data, chunk);
		}

		result += size;
	}
};

static char* node_to_buffer(pugi::xml_node node) {
	xml_memory_writer counter;
	node.print(counter);

	char* buffer = new char[counter.result + 1];

	xml_memory_writer writer(buffer, counter.result);
	node.print(writer);
	buffer[writer.written_size()] = 0;

	return buffer;
}

char* replaceAll(const char* in, const char* oldStr, const char* newStr) {
	int len = strlen(in);
	int nLen = strlen(newStr);
	int oLen = strlen(oldStr);

	if (len == 0)
		return new char[1]{0};

	char* res = new char[nLen <= oLen ? len + 1 : len * (nLen - oLen + 1)] {0};
	char* p = (char*)in;
	char* p2 = p;

	strncat(res, in, 0);

	while((p = strstr(p, oldStr))) {
		strncat(res, p2, p - p2);
		strncat(res, newStr, nLen);
		p = p + oLen;
		p2 = p;
	}

	strncat(res, p2, len - (p2 - in));
	return res;
}
/* End auxiliaries */

#define SUBSTITUTION  $$$SUBSTITUTION$$$

static void xml_valid(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL) 
		return sqlite3_result_null(ctx);
		
	const char* xml = (const char*)sqlite3_value_text(argv[0]);
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xml);
	return sqlite3_result_int(ctx, result ? 1 : 0);
}

static void xml_append(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[2]) == SQLITE_NULL) 
		return sqlite3_result_null(ctx);
		
	const char* xml = (const char*)sqlite3_value_text(argv[0]);
	const char* path = (const char*)sqlite3_value_text(argv[1]);
	const char* insertion = (const char*)sqlite3_value_text(argv[2]);
	char* res = 0;
	
	int FIRST = 1, BEFORE = 2, AFTER = 3, LAST = 4;
	int pos = AFTER;
	int isChild = false;
	if (argc > 3 && sqlite3_value_type(argv[3]) == SQLITE_TEXT) {
		const char* _pos = (const char*)sqlite3_value_text(argv[3]);
		isChild = strstr(_pos, "child") != 0; 		
		pos = strstr(_pos, "before") != 0 ? BEFORE : 
			strstr(_pos, "first") ? FIRST : 
			strstr(_pos, "last") ? LAST : 
			AFTER;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xml);
	if (!result)
		return sqlite3_result_error(ctx, "Incorrect XML", -1);	
	
	int isNode = 0;	
	try {
		pugi::xpath_node_set nodes = doc.select_nodes(path);
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
			isNode = (*it).node() != 0;
			
			if ((*it).node()) {
				pugi::xml_node node = (*it).node();
				pugi::xml_node pnode = (*it).node().parent();			
				if (isChild) {
					if (pos == FIRST || pos == BEFORE)
						node.prepend_child("$$$SUBSTITUTION$$$");
					else
						node.append_child("$$$SUBSTITUTION$$$");	
				} else {
					if (pos == FIRST)
						pnode.prepend_child("$$$SUBSTITUTION$$$");
					if (pos == BEFORE)
						pnode.insert_child_before("$$$SUBSTITUTION$$$", (*it).node());   
					if (pos == AFTER)
						pnode.insert_child_after("$$$SUBSTITUTION$$$", (*it).node());
					if (pos == LAST)
						pnode.append_child("$$$SUBSTITUTION$$$");	
				}					
			}
			
			if ((*it).attribute()) {
				pugi::xml_node node = (*it).parent();
				if (pos == FIRST)
					node.prepend_attribute("$$$SUBSTITUTION$$$");
				if (pos == BEFORE)
					node.insert_attribute_before("$$$SUBSTITUTION$$$", (*it).attribute());
				if (pos == AFTER)
					node.insert_attribute_after("$$$SUBSTITUTION$$$", (*it).attribute());
				if (pos == LAST)
					node.append_attribute("$$$SUBSTITUTION$$$");
			}
				
		}
		res = node_to_buffer(doc);
	} catch (pugi::xpath_exception& err) {
		res = (char*)malloc(sizeof(char) * (strlen(err.what()) + 1));
		sprintf(res, err.what());
	}
	
	if (res) {
		char* str = replaceAll(res, isNode ? "<$$$SUBSTITUTION$$$ />" : "$$$SUBSTITUTION$$$=\"\"", insertion);
		sqlite3_result_text(ctx, str, -1, SQLITE_TRANSIENT);
		delete [] str;
		delete [] res;
	} else {
		sqlite3_result_null(ctx);
	}	
}	

static void xml_extract(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[1]) == SQLITE_NULL) 
		return sqlite3_result_null(ctx);
		
	const char* xml = (const char*)sqlite3_value_text(argv[0]);
	const char* path = (const char*)sqlite3_value_text(argv[1]);	
	const char* sep = argc == 3 && sqlite3_value_type(argv[2]) == SQLITE_TEXT ? (const char*)sqlite3_value_text(argv[2]) : 0;	
	int sepLen = sep ? strlen(sep) : 0; 
	char* res = (char*)calloc(sizeof(char), sepLen + 1);
		
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xml);
	if (!result)
		return sqlite3_result_error(ctx, "Incorrect XML", -1);
	
	try {
		pugi::xpath_node_set nodes = doc.select_nodes(path);
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
			char* buf = 0;
			if ((*it).node()) 
				buf = node_to_buffer((*it).node());
			 
			if ((*it).attribute()) {
				const char* val = (*it).attribute().as_string();
				buf = (char*)malloc(sizeof(char) * (strlen(val) + 1));
				sprintf(buf, val); 
			}
			
			if (buf) {
				res = (char*)realloc(res, strlen(res) + strlen(buf) + sepLen + 1);
				if (strlen(res) > 0 && sepLen > 0)
					strcat(res, sep);
				strcat(res, buf);	
			}
		}
	} catch (pugi::xpath_exception& err) {
		delete [] res;
		res = (char*)malloc(sizeof(char) * (strlen(err.what()) + 1));
		sprintf(res, err.what());
	} 
	

	sqlite3_result_text(ctx, res, -1, SQLITE_TRANSIENT);		 
	delete [] res;
}

static void xml_remove(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL) 
		return sqlite3_result_null(ctx);
		
	const char* xml = (const char*)sqlite3_value_text(argv[0]);
	const char* path = (const char*)sqlite3_value_text(argv[1]);	
	char* res = 0;
		
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xml);
	if (!result)
		return sqlite3_result_error(ctx, "Incorrect XML", -1);
	
	try {
		pugi::xpath_node_set nodes = doc.select_nodes(path);
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
			if ((*it).node())
				(*it).node().parent().remove_child((*it).node());
			
			if ((*it).attribute())
				(*it).parent().remove_attribute((*it).attribute().name());
		}
		res = node_to_buffer(doc);
	} catch (pugi::xpath_exception& err) {
		res = (char*)malloc(sizeof(char) * (strlen(err.what()) + 1));
		sprintf(res, err.what());
	}
	
	if (res) {
		sqlite3_result_text(ctx, res, -1, SQLITE_TRANSIENT);		 
		delete [] res;
	} else {
		sqlite3_result_null(ctx);
	}	
}

static void xml_update(sqlite3_context *ctx, int argc, sqlite3_value **argv){
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL) 
		return sqlite3_result_null(ctx);
	
	if (sqlite3_value_type(argv[2]) == SQLITE_NULL)	
		return xml_remove(ctx, argc, argv);
		
	const char* xml = (const char*)sqlite3_value_text(argv[0]);
	const char* path = (const char*)sqlite3_value_text(argv[1]);	
	const char* replacement = (const char*)sqlite3_value_text(argv[2]);	
	char* res = 0;
		
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(xml);
	if (!result)
		return sqlite3_result_error(ctx, "Incorrect XML", -1);

	try {
		pugi::xpath_node_set nodes = doc.select_nodes(path);
		for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
			if ((*it).node()) {
				pugi::xml_node pnode = (*it).node().parent();
				pnode.insert_child_after("$$$SUBSTITUTION$$$", (*it).node());
				pnode.remove_child((*it).node());
			}
			
			if ((*it).attribute()) 
				(*it).parent().attribute((*it).attribute().name()) = replacement;
		}
		res = node_to_buffer(doc);
	} catch (pugi::xpath_exception& err) {
		res = (char*)malloc(sizeof(char) * (strlen(err.what()) + 1));
		sprintf(res, err.what());
	}
	
	if (res) {
		char* str = replaceAll(res, "<$$$SUBSTITUTION$$$ />", replacement);
		sqlite3_result_text(ctx, str, -1, SQLITE_TRANSIENT);
		delete [] str;
		delete [] res;
	} else {
		sqlite3_result_null(ctx);
	}
}

typedef struct xml_vtab xml_vtab;
struct xml_vtab {
	sqlite3_vtab base;
};

struct node_hash {
    size_t hash_value;
    int id;
    UT_hash_handle hh;
};

typedef struct xml_cursor xml_cursor;
struct xml_cursor {
	sqlite3_vtab_cursor base;
	
	sqlite3_int64 iRowid;
	int isEof; 
	char* xml;	
	char* path;
	node_hash* hashes;
	pugi::xml_document* doc;
	pugi::xpath_node_set nodes;
	pugi::xpath_node_set::const_iterator it;
};

static void hashNode(xml_cursor* pCur, pugi::xml_node* node, int* id) {
	node_hash *nh = new node_hash();
	int hash_value = node->hash_value();
    nh->hash_value = hash_value;
	nh->id = *id;
    HASH_ADD_INT(pCur->hashes, hash_value, nh);
	(*id)++;
		
	for (pugi::xml_node child: node->children())
		hashNode(pCur, &child, id);
}

static int xmlConnect(sqlite3 *db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVtab, char **pzErr){
	int rc = sqlite3_declare_vtab(db, "CREATE TABLE x(value text, pid integer, id integer, xml hidden, xpath hidden)");
	if (rc == SQLITE_OK) {
		xml_vtab* pTab = (xml_vtab*)sqlite3_malloc(sizeof(*pTab));
		*ppVtab = (sqlite3_vtab*)pTab;

		if (pTab == 0) 
			return SQLITE_NOMEM;

		memset(pTab, 0, sizeof(*pTab));
		sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
	}

	return rc;
}

static int xmlDisconnect(sqlite3_vtab *pVtab){
	xml_vtab *pTab = (xml_vtab*)pVtab;
	sqlite3_free(pTab);

	return SQLITE_OK;
}

static int xmlOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor){
 	xml_cursor* pCur = (xml_cursor*)sqlite3_malloc(sizeof(*pCur));
	if (pCur == 0)
		return SQLITE_NOMEM;

	memset(pCur, 0, sizeof(*pCur));
	*ppCursor = &pCur->base;
	pCur->xml = 0;
	pCur->path = 0;
	pCur->isEof = 1;
	pCur->doc = 0;
	pCur->hashes = 0;

	return SQLITE_OK;
}

static int xmlClose(sqlite3_vtab_cursor *cur){
	xml_cursor *pCur = (xml_cursor*)cur;
	if (pCur->xml)
		free(pCur->xml);
	if (pCur->path)
		free(pCur->path);
		
	if (pCur->doc)
		delete pCur->doc;	
		
	node_hash *nh, *tmp;
	HASH_ITER(hh, pCur->hashes, nh, tmp) {
		HASH_DEL(pCur->hashes, nh);
		delete nh;
	}		

	sqlite3_free(pCur);
	
	return SQLITE_OK;
}

static int xmlNext(sqlite3_vtab_cursor *cur){
	xml_cursor *pCur = (xml_cursor*)cur;
	pCur->it++;
	if (pCur->it == pCur->nodes.end())
		pCur->isEof = 1;
	
	pCur->iRowid++;
	return SQLITE_OK;
}

static int xmlColumn(sqlite3_vtab_cursor* cur, sqlite3_context* ctx, int colNo) {
	xml_cursor *pCur = (xml_cursor*)cur;
	xml_vtab *pTab = (xml_vtab*)(&pCur->base);
	
	auto getNodeId = [pCur](int hash_value) {
		struct node_hash *nh;
	
	    HASH_FIND_INT(pCur->hashes, &hash_value, nh);
	    return nh ? nh->id : -1;
	};
	
	if (colNo == 0) {
		char* buf = 0;
		pugi::xpath_node e = *(pCur->it);
		if (e.node()) 
			 buf = node_to_buffer((*pCur->it).node());
			 
		if (e.attribute()) {
			const char* val = e.attribute().as_string();
			buf = (char*)malloc(sizeof(char) * (strlen(val) + 1));
			sprintf(buf, val); 
		}
			
		if (buf) {	
			sqlite3_result_text(ctx, buf, -1, SQLITE_TRANSIENT);
			delete [] buf;
		} else {
			sqlite3_result_null(ctx);
		}
	} else if (colNo == 1) { // pid
		pugi::xpath_node e = *(pCur->it);
		int hash_value = e.node() ? e.parent().hash_value() : e.parent().parent().hash_value();
		sqlite3_result_int(ctx, getNodeId(hash_value));	
	} else if (colNo == 2) { // id
		pugi::xpath_node e = *(pCur->it);
		int hash_value = e.node() ? e.node().hash_value() : e.parent().hash_value();
		sqlite3_result_int(ctx, getNodeId(hash_value));	
	} else if (colNo == 3) {
		sqlite3_result_text(ctx, (char*)pCur->xml, -1, SQLITE_TRANSIENT);
	} else {
		sqlite3_result_text(ctx, pCur->path, -1, SQLITE_TRANSIENT);
	}
	return SQLITE_OK;

}

static int xmlRowid(sqlite3_vtab_cursor* cur, sqlite_int64* pRowid){
	xml_cursor *pCur = (xml_cursor*)cur;
	*pRowid = pCur->iRowid;

	return SQLITE_OK;
}

static int xmlEof(sqlite3_vtab_cursor *cur){
	xml_cursor *pCur = (xml_cursor*)cur;
	return pCur->isEof;
}

static int xmlFilter(sqlite3_vtab_cursor *cur, int idxNum, const char *idxStr, int argc, sqlite3_value **argv){
	xml_cursor *pCur = (xml_cursor *)cur;
	if (argc == 0) {
		pCur->isEof = 1;
		return SQLITE_OK;
	}
	
	if (sqlite3_value_type(argv[0]) != SQLITE_TEXT)
		return SQLITE_ERROR;
		
	if (pCur->xml == NULL) {		
		const char* xml = (const char*)sqlite3_value_text(argv[0]);
		pCur->xml = (char*)malloc(sizeof(char) * (strlen(xml) + 1));
		sprintf(pCur->xml, xml);
	
		pCur->doc = new pugi::xml_document();
		pugi::xml_parse_result result = pCur->doc->load_string(pCur->xml);
		if (!result)
			return SQLITE_ERROR;
		
		int id = 1;	
		hashNode(pCur, pCur->doc, &id);
		
		if (argc == 2 && sqlite3_value_type(argv[1]) == SQLITE_TEXT) {
			const char* path = (const char*)sqlite3_value_text(argv[1]);
			pCur->path = (char*)calloc(sizeof(char), strlen(path) + 1);
			sprintf(pCur->path, path);
		} else {
			pCur->path = (char*)calloc(sizeof(char), 1);
		}
		
		try {
			pCur->nodes = pCur->doc->select_nodes(strlen(pCur->path) ? pCur->path : "/");
		} catch (pugi::xpath_exception& err) {
			return SQLITE_ERROR;
		}
	}
		
	pCur->it = pCur->nodes.begin();		
	pCur->iRowid = 1;
	pCur->isEof = pCur->it == pCur->nodes.end();
	return SQLITE_OK;
}

static int xmlBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){	  
	if (!pIdxInfo->nConstraint)
		return SQLITE_RANGE;
			
	int i, j;
	int idxNum = 0;
	int unusableMask = 0;
	int nArg = 0;
	
	int nCol = 3; // id, pid, id
	int nHid = 2; // xml, xpath 
	int aIdx[nHid];
	
	for (i = 0; i < nHid; i++)
		aIdx[i] = -1;
		
	sqlite3_index_info::sqlite3_index_constraint* pConstraint;
	pConstraint = (sqlite3_index_info::sqlite3_index_constraint*)pIdxInfo->aConstraint;
	for(i = 0; i < pIdxInfo->nConstraint; i++, pConstraint++) {
		int iCol;
		int iMask;
		if(pConstraint->iColumn < nCol) 
			continue;
		
		iCol = pConstraint->iColumn - nCol;
		iMask = 1 << iCol;
		if(pConstraint->usable==0) {
			unusableMask |=  iMask;
			continue;
		} else if (pConstraint->op == SQLITE_INDEX_CONSTRAINT_EQ) {
			idxNum |= iMask;
			aIdx[iCol] = i;
		}
	}
	
	for(i = 0; i < nHid; i++) {
		if((j = aIdx[i]) >= 0){
			pIdxInfo->aConstraintUsage[j].argvIndex = ++nArg;
			pIdxInfo->aConstraintUsage[j].omit = 0;
		}
	}
	
	if((unusableMask & ~idxNum) != 0)
		return SQLITE_CONSTRAINT;
	
	pIdxInfo->estimatedCost = (double)100;
	pIdxInfo->estimatedRows = 100;
	pIdxInfo->idxNum = idxNum;
	return SQLITE_OK;
}

static sqlite3_module xmlModule = {
	/* iVersion    */ 0,
	/* xCreate     */ 0,
	/* xConnect    */ xmlConnect,
	/* xBestIndex  */ xmlBestIndex,
	/* xDisconnect */ xmlDisconnect,
	/* xDestroy    */ 0,
	/* xOpen       */ xmlOpen,
	/* xClose      */ xmlClose,
	/* xFilter     */ xmlFilter,
	/* xNext       */ xmlNext,
	/* xEof        */ xmlEof,
	/* xColumn     */ xmlColumn,
	/* xRowid      */ xmlRowid,
	/* xUpdate     */ 0,
	/* xBegin      */ 0,
	/* xSync       */ 0,
	/* xCommit     */ 0,
	/* xRollback   */ 0,
	/* xFindMethod */ 0,
	/* xRename     */ 0,
	/* xSavepoint  */ 0,
	/* xRelease    */ 0,
	/* xRollbackTo */ 0,
	/* xShadowName */ 0
};


extern "C"
{
#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_xml_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi) {
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);
	(void)pzErrMsg;  /* Unused parameter */
	return SQLITE_OK == sqlite3_create_function(db, "xml_valid", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_valid, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "xml_extract", 2, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_extract, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "xml_extract", 3, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_extract, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "xml_update", 3, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_update, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "xml_append", 3, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_append, 0, 0) &&
		SQLITE_OK == sqlite3_create_function(db, "xml_append", 4, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_append, 0, 0) &&		
		SQLITE_OK == sqlite3_create_function(db, "xml_remove", 2, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, xml_remove, 0, 0) &&
		SQLITE_OK == sqlite3_create_module(db, "xml_each", &xmlModule, 0) ?
		SQLITE_OK : SQLITE_ERROR;
}
}
