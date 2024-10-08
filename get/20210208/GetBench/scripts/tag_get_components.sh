#!/bin/bash
# $Id: tag_get_components.sh 1375 2014-12-01 13:12:29Z psizun $

# Default arguments
APPLY=${APPLY:-yes}
TAG=${TAG:-release-$(date +%Y%m%d)}

# Tag function
function tag_component()
{
	repo=$1
	component=$2
	tag=$3
	ticket=$4
	subdir=$5
	
	repourl="https://dsm-trac.cea.fr/svn/"${repo}
	comment="Creating tag '${tag}' of all GET software components; refs #${ticket}"
	
	src_url=${repourl}/${component}/trunk${subdir:+/${subdir}}
	dest_url=${repourl}/${component}/tags/${component}${subdir:+-${subdir}}-${tag}

	if test "$APPLY" != "yes"; then
		echo svn copy -m \"${comment}\" ${src_url} ${dest_url}
	else
		svn copy -m "${comment}" ${src_url} ${dest_url}
	fi
}

# Confirm
read -p  "Reading to create tag '${TAG}' of all components (yes/no)? " ans
if test "$ans" != "yes" ; then
	echo "Aborting"
	exit 1
fi

# Tag
tag_component cconfig TinyXml $TAG 189
tag_component cconfig CompoundConfig $TAG 189
tag_component cconfig ConfigWidgets $TAG 189
tag_component cconfig ConfigServer $TAG 189
tag_component cconfig soci $TAG 189
tag_component cconfig ConfigDatabase $TAG 189
tag_component cconfig DatabaseWidgets $TAG 189
tag_component mordicus Utilities $TAG 131
tag_component mordicus IceEirfu $TAG 131
tag_component mordicus MDaq $TAG 131
tag_component mordicus StateMachine $TAG 131
tag_component mordicus MultiFrame $TAG 131
tag_component get GetBench $TAG 60 software
tag_component get ConfigWizard $TAG 60
tag_component get GetController $TAG 60
tag_component get CoBoFrameViewer $TAG 60
#tag_component get AgetSerialController $TAG 60
#tag_component get AgetTestBenchLogAnalyzer $TAG 60
#tag_component get MutantAlignmentServer $TAG 60
