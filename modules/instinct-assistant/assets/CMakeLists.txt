cmake_minimum_required(VERSION 3.26)

file(GLOB_RECURSE ASSET_FILES *.sql)
cmrc_add_resource_library(instinct-assistants-resources ALIAS instinct::assistant::resources ${ASSET_FILES} NAMESPACE instinct::assistant)


