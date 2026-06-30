#include "pe.hpp"
#include "pe_bliss/pe_checksum.h"

void pe::pe_t::import_dll( const std::string_view dll, const std::span< const std::string > imports )
{
	const auto function_list = pe_bliss::get_imported_functions( image );

	if ( std::any_of( function_list.begin( ), function_list.end( ), [ &dll ] ( const pe_bliss::import_library& library ) {
		return library.get_name( ) == dll;
	} ) )
		return;

	pe_bliss::imported_functions_list build( function_list );

	pe_bliss::import_library library;
	library.set_name( dll.data( ) );

	auto counter = 0u;
	for ( const auto& imp : imports )
	{
		pe_bliss::imported_function func;
		func.set_name( imp );
		func.set_iat_va( ++counter );
	
		library.add_import( func );
	}

	build.push_back( library );

	pe_bliss::section new_section;
	new_section.get_raw_data( ).resize( 1 );
	new_section.set_name( ".rdata2" );
	new_section.readable( true ).writeable( true );

	auto& calculated = image.add_section( new_section );

	pe_bliss::import_rebuilder_settings settings( true, false );
	pe_bliss::rebuild_imports( image, build, calculated, settings );
}

pe::pe_t::pe_t( const std::filesystem::path& file )
	: 
	stream( file, std::ios::in | std::ios::out | std::ios::binary ),
	image( pe_bliss::pe_factory::create_pe( stream ) )
{

}

pe::pe_t::~pe_t( )
{
	// First, build to a temporary stringstream
	std::stringstream temp_stream;
	pe_bliss::rebuild_pe( image, temp_stream );
	
	// Seek temp_stream to beginning to calculate checksum
	temp_stream.seekg(0, std::ios::beg);
	uint32_t checksum = pe_bliss::calculate_checksum(temp_stream);
	
	// Set checksum to our image
	image.set_checksum(checksum);
	
	// Now rebuild again (with correct checksum) to the real file stream
	stream.seekp(0, std::ios::beg);
	pe_bliss::rebuild_pe(image, stream);
}
