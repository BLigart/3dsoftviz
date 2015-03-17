#include "Data/OsgEdge.h"

osg::ref_ptr<osg::Drawable> Data::OsgEdge::createLabel( QString name )
{
	label = new osgText::FadeText;
	label->setFadeSpeed( 0.03f );

	QString fontPath = Util::ApplicationConfig::get()->getValue( "Viewer.Labels.Font" );

	// experimental value
	float scale = 1.375f * this->type->getScale();

	if ( fontPath != NULL && !fontPath.isEmpty() ) {
		label->setFont( fontPath.toStdString() );
	}

	label->setText( name.toStdString() );
	label->setLineSpacing( 0 );
	label->setAxisAlignment( osgText::Text::SCREEN );
	label->setCharacterSize( scale );
	label->setDrawMode( osgText::Text::TEXT );
	label->setAlignment( osgText::Text::CENTER_BOTTOM_BASE_LINE );
	//label->setPosition((this->dstNode->getTargetPosition() + this->srcNode->getTargetPosition()) / 2 );
	label->setPosition( ( this->dstNode->restrictedTargetPosition() + this->srcNode->restrictedTargetPosition() ) / 2 );
	label->setColor( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

	return label;
}

osg::ref_ptr<osg::Drawable> Data::OsgEdge::createEdge( osg::StateSet* bbState )
{
	osg::ref_ptr<osg::Geometry> nodeQuad = new osg::Geometry;

	nodeQuad->setUseDisplayList( false );

	nodeQuad->setVertexArray( coordinates );
	nodeQuad->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS,0,4 ) );

	nodeQuad->setTexCoordArray( 0, edgeTexCoords );

	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array;
	colorArray->push_back( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

	osg::ref_ptr<osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType, 4, 1> > colorIndexArray = new osg::TemplateIndexArray<unsigned int, osg::Array::UIntArrayType, 4, 1>;
	colorIndexArray->push_back( 0 );

	nodeQuad->setColorArray( colorArray );
#ifdef BIND_PER_PRIMITIVE
	nodeQuad->setColorIndices( colorIndexArray );
#endif
	nodeQuad->setColorBinding( osg::Geometry::BIND_OVERALL );
	nodeQuad->setStateSet( bbState );

	return nodeQuad;
}

osg::ref_ptr<osg::StateSet> Data::OsgEdge::createStateSet( Data::Type* type )
{
	if ( !oriented ) {
		osg::StateSet* edgeStateSet = new osg::StateSet;

		edgeStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		edgeStateSet->setTextureAttributeAndModes( 0, Vwr::TextureWrapper::getEdgeTexture(), osg::StateAttribute::ON );
		edgeStateSet->setAttributeAndModes( new osg::BlendFunc, osg::StateAttribute::ON );
		edgeStateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

		edgeStateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

		osg::ref_ptr<osg::Depth> depth = new osg::Depth;
		depth->setWriteMask( false );
		edgeStateSet->setAttributeAndModes( depth, osg::StateAttribute::ON );

		return edgeStateSet;
	}
	else {
		osg::StateSet* orientedEdgeStateSet = new osg::StateSet;

		orientedEdgeStateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
		orientedEdgeStateSet->setTextureAttributeAndModes( 0, Vwr::TextureWrapper::getOrientedEdgeTexture(), osg::StateAttribute::ON );
		orientedEdgeStateSet->setMode( GL_BLEND, osg::StateAttribute::ON );
		orientedEdgeStateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::ON );

		orientedEdgeStateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
		osg::ref_ptr<osg::Depth> depth = new osg::Depth;
		depth->setWriteMask( false );
		orientedEdgeStateSet->setAttributeAndModes( depth, osg::StateAttribute::ON );

		return orientedEdgeStateSet;
	}
}