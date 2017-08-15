#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define INIT 0
#define PUSH 1
#define CLEAR 2
#define LENGTH 50
#define CMD_LEN 256

const char *remote;

const char* getprop(struct _xmlAttr *attr, char *prop) {
	while(attr != NULL) {
		if(strcmp(attr->name,prop)==0) {
			struct _xmlNode *node = attr->children;
			if(node==NULL) {
				printf("error get prop \"%s\" in %s\n", prop, attr->name);
				return NULL;
			}else {
				return node->content;
			}
		}
		attr = attr->next;
	}
	printf("err no prop \"%s\" in %s\n", prop, attr->name);
	return NULL;
}

int initGit(struct _xmlAttr *attr) {
	const char* prop = NULL;
	prop = getprop(attr,"name");
	if(prop!=NULL) {
		char buf[CMD_LEN]={0};
		sprintf(buf, "git init --bare --shared %s", prop);
		printf("execute: %s\n",buf);
		return system(buf);
	}
	return -1;	
}

int pushGit(struct _xmlAttr *attr) {
	const char* name = NULL;
	const char* path = NULL;
	char cur_path[CMD_LEN];
	int res;
	memset(cur_path, 0, CMD_LEN);
	getcwd(cur_path, CMD_LEN);
	name = getprop(attr,"name");
	path = getprop(attr,"path");
	if(name !=NULL && path !=NULL) {
		if((res=chdir(path))!=0) {
			printf("change to %s failed(%s)\n", path, strerror(errno));
			return res;
		}
		char buf[CMD_LEN];
		memset(buf, 0, CMD_LEN);
		sprintf(buf,"git remote | git fetch --unshallow `awk '{print $1}'`");
		if((res = system(buf))!=0) {
			printf("git remote --unshallow failed(%s), skip\n", name);
		}

		memset(buf, 0, CMD_LEN);
		sprintf(buf,"git remote add orig %s/%s", remote, name);
		if((res = system(buf))!=0) {
			printf("git add remote failed(%s)\n", name);
			goto out1;
		}

		memset(buf, 0, CMD_LEN);
		sprintf(buf,"git push orig HEAD:refs/heads/master");
		if((res = system(buf))!=0) {
			printf("git push failed(%s)\n", strerror(errno));
		}
	out1:
		chdir(cur_path);
	}
	return res;
}

int clearRemote(struct _xmlAttr *attr) {
	const char* path = NULL;
	char cur_path[CMD_LEN];
	int res;
	memset(cur_path, 0, CMD_LEN);
	getcwd(cur_path, CMD_LEN);
	path = getprop(attr,"path");
	if(path !=NULL) {
		if((res=chdir(path))!=0) {
			printf("change to %s failed(%s)\n", path, strerror(errno));
			return res;
		}
		char buf[CMD_LEN];
		memset(buf, 0, CMD_LEN);
		sprintf(buf,"git remote remove orig");
		if((res = system(buf))!=0) {
			printf("git remove remote failed(%s)\n", path);
		}
	out1:
		chdir(cur_path);
	}
	return res;
}

int parseXml(char *file,int action) {
	xmlDocPtr doc;
	xmlNodePtr cur;
	int res = -1;

	doc = xmlParseFile(file);
	if(doc == NULL) {
		printf("open %s failed\n",file);
		goto out1;
	}

	cur = xmlDocGetRootElement(doc);
	if(cur==NULL) {
		printf("get node failed\n");
		goto out2;
	}

	cur = cur->children;
	while(cur) {
		if(strcmp(cur->name,"project")==0) {
			struct _xmlAttr *attr = cur->properties;
			if(attr!=NULL) {
				switch (action) {
					case INIT:
						initGit(attr);
						break;
					case PUSH:
						pushGit(attr);
						break;
					case CLEAR:
						clearRemote(attr);
						break;
				}
			}
		}
		cur = cur->next;
	};

out2:
	xmlFreeDoc(doc);
out1:
	return res;
}

void printUsage(char *name) {
	printf("Usage: %s [option] <file1.xml> <file2.xml> ...... \n",name);
	printf("\toption :\n");
	printf("\t\t--init : execute \"git init\" at server\n");
	printf("\t\t--push <remote> : execute \"git push\" at client\n");
}

void main(int argc, char** argv) {

	int i=2;
	int ret=0;
	int action;

	if(argc < 3) {
		printUsage(argv[0]);
		return;
	}

	if(strcmp(argv[1],"--init")==0) {
		i=2;
		action = INIT;
	}else if(strcmp(argv[1],"--push")==0) {
		if(argc < 4) {
			printUsage(argv[0]);
			return;
		}
		action = PUSH;
		remote = argv[2];
		i=3;
	}else if(strcmp(argv[1],"--clear")==0) {
		action = CLEAR;
		i=2;
	}else {
		printUsage(argv[0]);
		return;
	}
	
	
	for(; i<argc; i++) {
#if DEBUG
		printf("%s\n",argv[i]);
#endif
		ret = parseXml(argv[i],action);
	}

}
