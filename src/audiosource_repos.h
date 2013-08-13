#ifndef __AUDIOSOURCE_REPOS_H__
#define __AUDIOSOURCE_REPOS_H__

#include "audiosource.h"

typedef struct AUDIOSOURCE_REPOS AUDIOSOURCE_REPOS;

AUDIOSOURCE_REPOS *audiosource_repos_create(AUDIOSOURCE *source, int position);

#endif
