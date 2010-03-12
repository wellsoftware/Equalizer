
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "eqPly.h"

#include "channel.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "view.h"
#include "window.h"

#include <stdlib.h>

using namespace eq::base;
using namespace std;

class NodeFactory : public eq::NodeFactory
{
public:
    virtual eq::Config*  createConfig( eq::ServerPtr parent )
        { return new eqPly::Config( parent ); }
    virtual eq::Node*    createNode( eq::Config* parent )  
        { return new eqPly::Node( parent ); }
    virtual eq::Pipe*    createPipe( eq::Node* parent )
        { return new eqPly::Pipe( parent ); }
    virtual eq::Window*  createWindow( eq::Pipe* parent )
        { return new eqPly::Window( parent ); }
    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new eqPly::Channel( parent ); }
    virtual eq::View* createView( eq::Layout* parent )
        { return new eqPly::View( parent ); }
};

int main( const int argc, char** argv )
{
    // 1. parse arguments
    eqPly::LocalInitData initData;
    initData.parseArguments( argc, argv );

    // 2. Redirect log output to file, if requested
    std::ofstream* logFile = 0;
    const std::string& logFilename = initData.getLogFilename();
    if( !logFilename.empty( ))
    {
        logFile = new ofstream( logFilename.c_str( ));
        eq::base::Log::setOutput( *logFile );
    }
    
    // 3. Equalizer initialization
    NodeFactory nodeFactory;
    if( !eq::init( argc, argv, &nodeFactory ))
    {
        EQERROR << "Equalizer init failed" << endl;
        return EXIT_FAILURE;
    }
    
    // 4. initialization of local client node
    RefPtr< eqPly::EqPly > client = new eqPly::EqPly( initData );
    if( !client->initLocal( argc, argv ))
    {
        EQERROR << "Can't init client" << endl;
        eq::exit();
        return EXIT_FAILURE;
    }

    // 5. run client
    const int ret = client->run();

    // 6. cleanup and exit
    client->exitLocal();

    // TODO EQASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
    client = 0;

    eq::exit();

    if( logFile )
        logFile->close();
    delete logFile;

    return ret;
}
