/*
 * Copyright 2008 Arsen Chaloyan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mrcp_resource_loader.h"
#include "mrcp_resource_factory.h"
#include "mrcp_synth_resource.h"
#include "mrcp_recog_resource.h"
#include "mrcp_recorder_resource.h"
#include "apt_log.h"

/** Resource loader */
struct mrcp_resource_loader_t {
	mrcp_resource_factory_t *factory;
	apr_pool_t              *pool;
};

/** String table of MRCPv2 resources (mrcp_resource_types_e) */
static const apt_str_table_item_t mrcp_resource_string_table[] = {
	{{"speechsynth",11},6},
	{{"speechrecog",11},6},
	{{"recorder",    8},0}
};

/** Create default MRCP resource factory */
MRCP_DECLARE(mrcp_resource_loader_t*) mrcp_resource_loader_create(apt_bool_t load_all_resources, apr_pool_t *pool)
{
	mrcp_resource_loader_t *loader;
	mrcp_resource_factory_t *resource_factory;
	resource_factory = mrcp_resource_factory_create(MRCP_RESOURCE_TYPE_COUNT,pool);
	if(!resource_factory) {
		return NULL;
	}
	
	/* set resource string table */
	mrcp_resource_string_table_set(resource_factory,mrcp_resource_string_table);

	loader = apr_palloc(pool,sizeof(mrcp_resource_loader_t));
	loader->factory = resource_factory;
	loader->pool = pool;

	if(load_all_resources == TRUE) {
		mrcp_resources_load(loader);
	}

	return loader;
}

/** Load all MRCP resources */
MRCP_DECLARE(apt_bool_t) mrcp_resources_load(mrcp_resource_loader_t *loader)
{
	mrcp_resource_id id;
	for(id=0; id<MRCP_RESOURCE_TYPE_COUNT; id++) {
		mrcp_resource_load_by_id(loader,id);
	}
	return TRUE;
}

/** Load MRCP resource by resource name */
MRCP_DECLARE(apt_bool_t) mrcp_resource_load(mrcp_resource_loader_t *loader, const apt_str_t *name)
{
	mrcp_resource_id id = mrcp_resource_id_find(loader->factory,name);
	return mrcp_resource_load_by_id(loader,id);
}

/** Load MRCP resource by resource identifier */
MRCP_DECLARE(apt_bool_t) mrcp_resource_load_by_id(mrcp_resource_loader_t *loader, mrcp_resource_id id)
{
	const apt_str_t *name;
	mrcp_resource_t *resource = NULL;
	switch(id) {
		case MRCP_SYNTHESIZER_RESOURCE:
			resource = mrcp_synth_resource_create(loader->pool);
			break;
		case MRCP_RECOGNIZER_RESOURCE:
			resource = mrcp_recog_resource_create(loader->pool);
			break;
		case MRCP_RECORDER_RESOURCE:
			resource = mrcp_recorder_resource_create(loader->pool);
			break;
	}

	if(!resource) {
		apt_log(APT_LOG_MARK,APT_PRIO_NOTICE,"Failed to Load Register [%d]",id);
		return FALSE;
	}

	name = mrcp_resource_name_get(loader->factory,id);
	if(!name) {
		apt_log(APT_LOG_MARK,APT_PRIO_NOTICE,"Failed to Load Register [%d]",id);
		return FALSE;
	}
	
	apt_log(APT_LOG_MARK,APT_PRIO_NOTICE,"Register Resource [%s]",name->buf);
	return mrcp_resource_register(loader->factory,resource,id);
}

/** Get MRCP resource factory */
MRCP_DECLARE(mrcp_resource_factory_t*) mrcp_resource_factory_get(mrcp_resource_loader_t *loader)
{
	return loader->factory;
}