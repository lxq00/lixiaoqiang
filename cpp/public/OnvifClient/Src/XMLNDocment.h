#ifndef __XMLNDOCMENT_H__
#define __XMLNDOCMENT_H__
#include "Base/Base.h"
using namespace Public::Base;

typedef struct XMLN
{
	const char *	name;
	unsigned int	type;
	const char *	data;
	int				dlen;
	int				finish;
	struct XMLN *	parent;
	struct XMLN *	f_child;
	struct XMLN *	l_child;
	struct XMLN *	prev;
	struct XMLN *	next;
	struct XMLN *	f_attrib;
	struct XMLN *	l_attrib;
}XMLN;

#define NTYPE_TAG		0
#define NTYPE_ATTRIB	1
#define NTYPE_CDATA		2

#define NTYPE_LAST		2
#define NTYPE_UNDEF		-1


static XMLN * xml_node_soap_get(XMLN * parent, const char * name)
{
	if (parent == NULL || name == NULL)
		return NULL;

	XMLN * p_node = parent->f_child;
	while (p_node != NULL)
	{
		if (soap_strcmp(p_node->name, name) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

static int soap_strcmp(const char * str1, const char * str2)
{
	if (strcmp(str1, str2) == 0)
		return 0;

	const char * ptr1 = strchr(str1, ':');
	const char * ptr2 = strchr(str2, ':');
	if (ptr1 && ptr2)
		return strcmp(ptr1 + 1, ptr2 + 1);
	else if (ptr1)
		return strcmp(ptr1 + 1, str2);
	else if (ptr2)
		return strcmp(str1, ptr2 + 1);
	else
		return -1;
}

static const char * xml_attr_get(XMLN * p_node, const char * name)
{
	if (p_node == NULL || name == NULL)
		return NULL;

	XMLN * p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		//		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcasecmp(p_attr->name,name)))
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == strcmp(p_attr->name, name)))
			return p_attr->data;

		p_attr = p_attr->next;
	}

	return NULL;
}

#define LTXML_MAX_STACK_DEPTH	1024
#define LTXML_MAX_ATTR_NUM	128
typedef struct ltxd_xmlparser
{
	char *	xmlstart;
	char *	xmlend;
	char *	ptr;		// pointer to current character
	int		xmlsize;

	char *	e_stack[LTXML_MAX_STACK_DEPTH];
	int		e_stack_index;

	const char *	attr[LTXML_MAX_ATTR_NUM];

	void *	userdata;
	void(*startElement)(void * userdata, const char * name, const char ** attr);
	void(*endElement)(void * userdata, const char * name);
	void(*charData)(void * userdata, const char * str, int len);
}LTXMLPRS;

/***********************************************************************/
XMLN * xml_node_add(XMLN * parent, char * name)
{
	XMLN * p_node = (XMLN *)malloc(sizeof(XMLN));
	if (p_node == NULL)
	{
		//log_print("xml_node_add::memory alloc fail!!!\r\n");
		return NULL;
	}

	memset(p_node, 0, sizeof(XMLN));

	p_node->type = NTYPE_TAG;
	p_node->name = name;	//strdup(name);

	if (parent != NULL)
	{
		p_node->parent = parent;
		if (parent->f_child == NULL)
		{
			parent->f_child = p_node;
			parent->l_child = p_node;
		}
		else
		{
			parent->l_child->next = p_node;
			p_node->prev = parent->l_child;
			parent->l_child = p_node;
		}
	}

	return p_node;
}

/*---------------------------------------------------------------------*/
XMLN * xml_attr_add(XMLN * p_node, const char * name, const char * value)
{
	if (p_node == NULL || name == NULL || value == NULL)
		return NULL;

	XMLN * p_attr = (XMLN *)malloc(sizeof(XMLN));
	if (p_attr == NULL)
	{
		//log_print("xml_attr_add::memory alloc fail!!!\r\n");
		return NULL;
	}

	memset(p_attr, 0, sizeof(XMLN));

	p_attr->type = NTYPE_ATTRIB;
	p_attr->name = name;	//strdup(name);
	p_attr->data = value;	//strdup(value);
	p_attr->dlen = strlen(value);

	if (p_node->f_attrib == NULL)
	{
		p_node->f_attrib = p_attr;
		p_node->l_attrib = p_attr;
	}
	else
	{
		p_attr->prev = p_node->l_attrib;
		p_node->l_attrib->next = p_attr;
		p_node->l_attrib = p_attr;
	}

	return p_attr;
}

void xml_node_del(XMLN * p_node)
{
	if (p_node == NULL)return;

	XMLN * p_attr = p_node->f_attrib;
	while (p_attr)
	{
		XMLN * p_next = p_attr->next;
		//	if(p_attr->data) free(p_attr->data);
		//	if(p_attr->name) free(p_attr->name);

		free(p_attr);

		p_attr = p_next;
	}

	XMLN * p_child = p_node->f_child;
	while (p_child)
	{
		XMLN * p_next = p_child->next;
		xml_node_del(p_child);
		p_child = p_next;
	}

	if (p_node->prev) p_node->prev->next = p_node->next;
	if (p_node->next) p_node->next->prev = p_node->prev;

	if (p_node->parent)
	{
		if (p_node->parent->f_child == p_node)
			p_node->parent->f_child = p_node->next;
		if (p_node->parent->l_child == p_node)
			p_node->parent->l_child = p_node->prev;
	}

	//	if(p_node->name) free(p_node->name);
	//	if(p_node->data) free(p_node->data);

	free(p_node);
}
void xml_attr_del(XMLN * p_node, const char * name)
{
	if (p_node == NULL || name == NULL)
		return;

	XMLN * p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		//		if(strcasecmp(p_attr->name,name) == 0)
		if (strcmp(p_attr->name, name) == 0)
		{
			xml_node_del(p_attr);
			return;
		}

		p_attr = p_attr->next;
	}
}
void stream_startElement(void * userdata, const char * name, const char ** atts)
{
	XMLN ** pp_node = (XMLN **)userdata;
	if (pp_node == NULL)
	{
		return;
	}

	XMLN * parent = *pp_node;
	XMLN * p_node = xml_node_add(parent, (char *)name);
	if (atts)
	{
		int i = 0;
		while (atts[i] != NULL)
		{
			if (atts[i + 1] == NULL)
				break;

			/*XMLN * p_attr = */xml_attr_add(p_node, atts[i], atts[i + 1]);	//(XMLN *)malloc(sizeof(XMLN));

			i += 2;
		}
	}

	*pp_node = p_node;
}

void stream_endElement(void * userdata, const char * name)
{
	XMLN ** pp_node = (XMLN **)userdata;
	if (pp_node == NULL)
	{
		return;
	}

	XMLN * p_node = *pp_node;
	if (p_node == NULL)
	{
		return;
	}

	p_node->finish = 1;

	if (p_node->type == NTYPE_TAG && p_node->parent == NULL)
	{
		// parse finish
	}
	else
	{
		*pp_node = p_node->parent;	// back up a level
	}
}

void stream_charData(void* userdata, const char* s, int len)
{
	XMLN ** pp_node = (XMLN **)userdata;
	if (pp_node == NULL)
	{
		return;
	}

	//	const char *pchar = NULL;
	XMLN * p_node = *pp_node;
	if (p_node == NULL)
	{
		//	log_user_print((void *)p_user,"stream_charData::cur_node is null,s'addr=0x%x,len=%d!!!\r\n",s,len);
		return;
	}
	p_node->data = s;
	p_node->dlen = len;
}

#define IS_WHITE_SPACE(c) ((c==' ') || (c=='\t') || (c=='\r') || (c=='\n'))

#define IS_XMLH_START(ptr) (*ptr == '<' && *(ptr+1) == '?')
#define IS_XMLH_END(ptr) (*ptr == '?' && *(ptr+1) == '>')

// <?xml version="1.0" encoding="UTF-8"?>
int hxml_parse_header(LTXMLPRS * parse)
{
	char * ptr = parse->ptr;
	char * xmlend = parse->xmlend;

	while (IS_WHITE_SPACE(*ptr) && (ptr != xmlend)) ptr++;
	if (ptr == xmlend) return -1;

	if (!IS_XMLH_START(ptr))
		return -1;
	ptr += 2;
	while ((!IS_XMLH_END(ptr)) && (ptr != xmlend)) ptr++;
	if (ptr == xmlend) return -1;
	ptr += 2;
	parse->ptr = ptr;

	return 0;
}


#define IS_ELEMS_END(ptr) (*ptr == '>' || (*ptr == '/' && *(ptr+1) == '>'))
#define IS_ELEME_START(ptr) (*ptr == '<' && *(ptr+1) == '/')

#define RF_NO_END		0
#define RF_ELES_END		2
#define RF_ELEE_END		3

int hxml_parse_attr(LTXMLPRS * parse)
{
	char * ptr = parse->ptr;
	char * xmlend = parse->xmlend;
	int ret = RF_NO_END;
	int cnt = 0;

	while (1)
	{
		ret = RF_NO_END;

		while (IS_WHITE_SPACE(*ptr) && (ptr != xmlend)) ptr++;
		if (ptr == xmlend) return -1;

		if (*ptr == '>')
		{
			*ptr = '\0'; ptr++;
			ret = RF_ELES_END; // node start finish
			break;
		}
		else if (*ptr == '/' && *(ptr + 1) == '>')
		{
			*ptr = '\0'; ptr += 2;
			ret = RF_ELEE_END;	// node end finish
			break;
		}

		char * attr_name = ptr;
		while (*ptr != '=' && (!IS_ELEMS_END(ptr)) && ptr != xmlend) ptr++;
		if (ptr == xmlend) return -1;
		if (IS_ELEMS_END(ptr))
		{
			if (*ptr == '>')
			{
				ret = RF_ELES_END;
				*ptr = '\0'; ptr++;
			}
			else if (*ptr == '/' && *(ptr + 1) == '>')
			{
				ret = RF_ELEE_END;
				*ptr = '\0'; ptr += 2;
			}
			break;
		}

		*ptr = '\0';	// '=' --> '\0'
		ptr++;

		char * attr_value = ptr;
		if (*ptr == '"')
		{
			attr_value++;
			ptr++;
			while (*ptr != '"' && ptr != xmlend) ptr++;
			if (ptr == xmlend) return -1;
			*ptr = '\0'; // '"' --> '\0'

			ptr++;
		}
		else
		{
			while ((!IS_WHITE_SPACE(*ptr)) && (!IS_ELEMS_END(ptr)) && ptr != xmlend) ptr++;
			if (ptr == xmlend) return -1;

			if (IS_WHITE_SPACE(*ptr))
			{
				*ptr = '\0';
				ptr++;
			}
			else
			{
				if (*ptr == '>')
				{
					ret = RF_ELES_END;
					*ptr = '\0'; ptr++;
				}
				else if (*ptr == '/' && *(ptr + 1) == '>')
				{
					ret = RF_ELEE_END;
					*ptr = '\0'; ptr += 2;
				}
			}
		}

		int index = cnt << 1;
		parse->attr[index] = attr_name;
		parse->attr[index + 1] = attr_value;
		cnt++;
		if (ret > RF_NO_END)
			break;
	}

	parse->ptr = ptr;
	return ret;
}


#define CHECK_XML_STACK_RET(parse) \
	do{ if(parse->e_stack_index >= LTXML_MAX_STACK_DEPTH || parse->e_stack_index < 0) return -1;}while(0)

int hxml_parse_element_end(LTXMLPRS * parse)
{
	char * stack_name = parse->e_stack[parse->e_stack_index];
	if (stack_name == NULL)
		return -1;

	char * xmlend = parse->xmlend;

	while (IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
	if ((parse->ptr) == xmlend) return -1;

	char * end_name = (parse->ptr);
	while ((!IS_WHITE_SPACE(*(parse->ptr))) && ((parse->ptr) != xmlend) && (*(parse->ptr) != '>')) (parse->ptr)++;
	if ((parse->ptr) == xmlend) return -1;
	if (IS_WHITE_SPACE(*(parse->ptr)))
	{
		*(parse->ptr) = '\0'; (parse->ptr)++;
		while (IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
		if ((parse->ptr) == xmlend) return -1;
	}

	if (*(parse->ptr) != '>') return -1;
	*(parse->ptr) = '\0';
	(parse->ptr)++;

	//	if(strcasecmp(end_name, stack_name) != 0)
	if (strcmp(end_name, stack_name) != 0)
	{
		//log_print("hxml_parse_element_end::cur name[%s] != stack name[%s]!!!\r\n", end_name, stack_name);
		return -1;
	}

	if (parse->endElement)
		parse->endElement(parse->userdata, end_name);

	parse->e_stack[parse->e_stack_index] = NULL; parse->e_stack_index--;
	CHECK_XML_STACK_RET(parse);

	return 0;
}


int hxml_parse_element_start(LTXMLPRS * parse)
{
	char * xmlend = parse->xmlend;

	while (IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
	if ((parse->ptr) == xmlend) return -1;

	char * element_name = (parse->ptr);
	while ((!IS_WHITE_SPACE(*(parse->ptr))) && ((parse->ptr) != xmlend) && (!IS_ELEMS_END((parse->ptr)))) (parse->ptr)++;
	if ((parse->ptr) == xmlend) return -1;

	parse->e_stack_index++; parse->e_stack[parse->e_stack_index] = element_name;
	CHECK_XML_STACK_RET(parse);

	if (*(parse->ptr) == '>')
	{
		*(parse->ptr) = '\0'; (parse->ptr)++;
		if (parse->startElement)
			parse->startElement(parse->userdata, element_name, parse->attr);
		return RF_ELES_END;
	}
	else if (*(parse->ptr) == '/' && *((parse->ptr) + 1) == '>')
	{
		*(parse->ptr) = '\0'; (parse->ptr) += 2;
		if (parse->startElement)
			parse->startElement(parse->userdata, element_name, parse->attr);
		if (parse->endElement)
			parse->endElement(parse->userdata, element_name);

		parse->e_stack[parse->e_stack_index] = NULL; parse->e_stack_index--;
		CHECK_XML_STACK_RET(parse);
		return RF_ELEE_END;
	}
	else
	{
		*(parse->ptr) = '\0'; (parse->ptr)++;

		int ret = hxml_parse_attr(parse); if (ret < 0) return -1;

		if (parse->startElement)
			parse->startElement(parse->userdata, element_name, parse->attr);

		memset(parse->attr, 0, sizeof(parse->attr));

		if (ret == RF_ELEE_END)
		{
			if (parse->endElement)
				parse->endElement(parse->userdata, element_name);

			parse->e_stack[parse->e_stack_index] = NULL; parse->e_stack_index--;
			CHECK_XML_STACK_RET(parse);
		}

		return ret;
	}
}

#define CUR_PARSE_START		1
#define CUR_PARSE_END		2

int hxml_parse_element(LTXMLPRS * parse)
{
	char * xmlend = parse->xmlend;
	int parse_type = CUR_PARSE_START;
	while (1)
	{
		int ret = RF_NO_END;

	xml_parse_type:
		while (IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
		if ((parse->ptr) == xmlend)
		{
			if (parse->e_stack_index == 0)
				return 0;
			return -1;
		}


		if (*(parse->ptr) == '<' && *((parse->ptr) + 1) == '/')
		{
			(parse->ptr) += 2;
			parse_type = CUR_PARSE_END;
		}
		else if (*(parse->ptr) == '<')
		{
			(parse->ptr)++;
			parse_type = CUR_PARSE_START;
		}
		else
		{
			return -1;
		}

		//xml_parse_point:

		while (IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
		if ((parse->ptr) == xmlend)
		{
			if (parse->e_stack_index == 0)
				return 0;
			return -1;
		}

		if (parse_type == CUR_PARSE_END)
		{
			ret = hxml_parse_element_end(parse);
			if (ret < 0)
				return -1;

			if (parse->e_stack_index == 0)
				return 0;

			parse_type = CUR_PARSE_START;
		}
		else // CUR_PARSE_START
		{
			ret = hxml_parse_element_start(parse);
			if (ret < 0)
				return -1;
			if (ret == RF_ELEE_END)
			{
				if (parse->e_stack_index == 0)
					return 0;

				parse_type = CUR_PARSE_START;
				goto xml_parse_type;
			}

			while (IS_WHITE_SPACE(*(parse->ptr)) && ((parse->ptr) != xmlend)) (parse->ptr)++;
			if ((parse->ptr) == xmlend) return -1;

			if (*(parse->ptr) == '<') goto xml_parse_type;

			char * cdata_ptr = (parse->ptr);
			while (*(parse->ptr) != '<' && (parse->ptr) != xmlend) (parse->ptr)++;
			if ((parse->ptr) == xmlend) return -1;

			int len = (parse->ptr) - cdata_ptr;
			if (len > 0)
			{
				*(parse->ptr) = '\0'; (parse->ptr)++;
				if (parse->charData)
					parse->charData(parse->userdata, cdata_ptr, len);

				if (*(parse->ptr) != '/')
					return -1;

				(parse->ptr)++;

				if (hxml_parse_element_end(parse) < 0)
					return -1;
			}

			goto xml_parse_type;
		}
	}

	return 0;
}

int hxml_parse(LTXMLPRS * parse)
{
	// //log_print("hxml parse header start...\r\n");
	int ret = hxml_parse_header(parse);
	if (ret < 0)
	{
		// //log_print("hxml parse xml header failed!!!\r\n");
		// return -1;
	}

	// //log_print("hxml parse element start...\r\n");
	return hxml_parse_element(parse);
}

XMLN * xxx_hxml_parse(char * p_xml, int len)
{
	XMLN * p_root = NULL;

	LTXMLPRS parse;
	memset(&parse, 0, sizeof(parse));

	parse.userdata = &p_root;
	parse.startElement = stream_startElement;
	parse.endElement = stream_endElement;
	parse.charData = stream_charData;

	parse.xmlstart = p_xml;
	parse.xmlend = p_xml + len;
	parse.ptr = parse.xmlstart;

	int status = hxml_parse(&parse);
	if (status < 0)
	{
		//log_print("xxx_hxml_parse::err[%d]\r\n", status);

		xml_node_del(p_root);
		p_root = NULL;
	}

	return p_root;
}

#endif