/*
    The MIT License (MIT)

    Copyright (c) 2016 Benoit AUTHEMAN

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

//-----------------------------------------------------------------------------
// This file is a part of the GTpo software library.
//
// \file	gtpoGmlSerializer.hpp
// \author	benoit@qanava.org
// \date	2016 01 31
//-----------------------------------------------------------------------------

#include <cassert>
#include <stdexcept>

namespace gtpo { // ::gtpo

template < class GraphConfig, class Notifier >
OutGmlSerializer< GraphConfig, Notifier >::OutGmlSerializer( std::string xmlFileName ) :
    _xmlFileName( xmlFileName )
{
    assert( xmlFileName.length() > 0 );
}

template < class GraphConfig, class Notifier >
void    OutGmlSerializer< GraphConfig, Notifier >::serializeOut( Graph& graph, Notifier* notifier )
{
    ( void )notifier;
    try {
        auto graphmlNode = _xmlDoc.append_child( "graphml" );
        if ( !graphmlNode )
            throw std::exception();
        auto graphNode = graphmlNode.append_child( "graph" );
        if ( !graphNode )
            throw std::exception();
        graphNode.append_attribute( "id" );
        graphNode.append_attribute( "edgedefault" );
        _xmlCurrentParent = graphNode;

        // Serialize GTpo default attributes
        serializeAttribute( graphNode, "x", "node", "double", "0." );
        serializeAttribute( graphNode, "y", "node", "double", "0." );
        serializeAttribute( graphNode, "width", "node", "double", "0." );
        serializeAttribute( graphNode, "height", "node", "double", "0." );
        serializeAttribute( graphNode, "serializable", "node", "bool", "true" );
        serializeAttribute( graphNode, "weight", "edge", "double", "0." );

        // FIXME 20160208 virer getNodes()
        for ( auto node : graph.getNodes() )
            serializeNode( node );
        for ( auto edge : graph.getEdges() )
            serializeEdge( edge );
    } catch ( std::exception const& ) {

    }
}

template < class GraphConfig, class Notifier >
void    OutGmlSerializer< GraphConfig, Notifier >::serializeNode( const WeakNode& node )
{
    SharedNode sharedNode = node.lock();
    if ( !sharedNode )
        throw std::runtime_error( "gtpo::OutGmlPugiSerializer<>::serializeNode(): can't lock node." );
    if ( !_xmlCurrentParent )
        throw std::exception();
    auto nodeNode = _xmlCurrentParent.append_child( "node" );
    if ( !nodeNode )
        throw std::exception();
    auto idAttr = nodeNode.append_attribute( "id" );
    idAttr.set_value( std::to_string( reinterpret_cast<uint64_t>( sharedNode.get() ) ).c_str() );
    serializeNodeAttribute( nodeNode, "x", std::to_string( GraphConfig::getNodeX( sharedNode ) ) );
    serializeNodeAttribute( nodeNode, "y", std::to_string( GraphConfig::getNodeY( sharedNode ) ) );
    serializeNodeAttribute( nodeNode, "width", std::to_string( GraphConfig::getNodeWidth( sharedNode ) ) );
    serializeNodeAttribute( nodeNode, "height", std::to_string( GraphConfig::getNodeHeight( sharedNode ) ) );
}

template < class GraphConfig, class Notifier >
void    OutGmlSerializer< GraphConfig, Notifier >::serializeNodeAttribute( pugi::xml_node& nodeNode, const std::string& id, const std::string& value )
{
    auto dataNode = nodeNode.append_child( "data" );
    if ( !dataNode )
        throw std::runtime_error( "gtpo::OutGmlPugiSerializer<>::serializeNodeAttribute(): error while creating node data sub node." );
    dataNode.set_name( "data" );
    auto idAttr = dataNode.append_attribute( "id" );
    idAttr.set_value( id.c_str() );
    dataNode.append_child( pugi::node_pcdata ).set_value( value.c_str() );
}

template < class GraphConfig, class Notifier >
void    OutGmlSerializer< GraphConfig, Notifier >::serializeEdge( const WeakEdge& edge ) noexcept( false )
{
    SharedEdge sharedEdge = edge.lock();
    gtpo::assert_throw( sharedEdge != nullptr, "gtpo::OutGmlPugiSerializer<>::serializeEdge(): Error: Can't lock edge." );
    gtpo::assert_throw( _xmlCurrentParent, "gtpo::OutGmlPugiSerializer<>::serializeEdge(): Error: Invalid XML parent node." );
    auto edgeNode = _xmlCurrentParent.append_child( "edge" );
    gtpo::assert_throw( edgeNode, "gtpo::OutGmlPugiSerializer<>::serializeEdge(): Error: Can't create edge XML node." );

    auto idAttr = edgeNode.append_attribute( "id" );
    idAttr.set_value( std::to_string( reinterpret_cast<uint64_t>( sharedEdge.get() ) ).c_str() );

    SharedNode ownedSource = sharedEdge->getSrc().lock();
    SharedNode ownedDestination = sharedEdge->getDst().lock();
    gtpo::assert_throw( ownedSource && ownedDestination, "gtpo::OutGmlPugiSerializer<>::serializeEdge(): Error: Can't lock edge source or destination." );
    auto sourceAttr = edgeNode.append_attribute( "source" );
    sourceAttr.set_value( std::to_string( reinterpret_cast<uint64_t>( ownedSource.get() ) ).c_str() );
    auto targetAttr = edgeNode.append_attribute( "target" );
    targetAttr.set_value( std::to_string( reinterpret_cast<uint64_t>( ownedDestination.get() ) ).c_str() );

}

template < class GraphConfig, class Notifier >
void    OutGmlSerializer< GraphConfig, Notifier >::serializeAttribute( pugi::xml_node& graphNode, const std::string& name, const std::string& target,
                                                             const std::string& type, const std::string& def )
{
    auto keyNode = graphNode.append_child( "key" );
    if ( !keyNode )
        throw std::runtime_error( "gtpo::OutGmlPugiSerializer<>::serializeAttribute(): invalid key node." );
    auto idAttr = keyNode.append_attribute( "id" );
    idAttr.set_value( name.c_str() );
    auto forAttr = keyNode.append_attribute( "for" );
    forAttr.set_value( target.c_str() );
    auto nameAttr = keyNode.append_attribute( "attr.name" );
    nameAttr.set_value( name.c_str() );
    auto typeAttr = keyNode.append_attribute( "attr.type" );
    typeAttr.set_value( type.c_str() );
    auto defaultNode = keyNode.append_child( "default" );
    if ( !defaultNode )
        throw std::runtime_error( "gtpo::OutGmlPugiSerializer<>::serializeAttribute(): error creating attribute key node default sub node." );
    defaultNode.set_name( "default" );
    defaultNode.append_child( pugi::node_pcdata ).set_value( def.c_str() );
}

template < class GraphConfig, class Notifier >
void    OutGmlSerializer< GraphConfig, Notifier >::finishOut( )
{
    assert( _xmlFileName.length() > 0 );
    _xmlDoc.save_file( _xmlFileName.c_str() );
}

template < class GraphConfig, class Notifier >
InGmlSerializer< GraphConfig, Notifier >::InGmlSerializer( std::string fileName ) :
    _fileName( fileName )
{
    std::assert( fileName.length() > 0 );
    pugi::xml_parse_result result = _xmlDoc.load_file( fileName.c_str() );
    if ( !result ) {
        std::cout << "qtpo::InGmlPugiSerializer<>::InGmlPugiSerializer(): XML " << fileName.c_str() << " parsed with errors" << std::endl;
        std::cout << "\tDescription: " << result.description() << std::endl;
        std::cout << "\tOffset: " << result.offset << std::endl;
        throw std::expection;
    }
}

template < class GraphConfig, class Notifier >
void    InGmlSerializer< GraphConfig, Notifier >::serializeIn( Graph& graph, Notifier* notifier )
{
    try {
        for ( pugi::xml_node xmlNode: tools.children("node") ) {
            auto node = graph.createNode();
            serializeNode( node );
        }
    } catch ( std::exception const& ) { }
}

template < class GraphConfig, class Notifier >
void    InGmlSerializer< GraphConfig, Notifier >::serializeNode( Node& node )
{
    if ( !_xmlCurrentNode )
        throw std::exception;

}

template < class GraphConfig, class Notifier >
void    InGmlSerializer< GraphConfig, Notifier >::finishIn( )
{
    // No cleanups necessary
}

} // ::gtpo
