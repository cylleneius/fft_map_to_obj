#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
using namespace std;

typedef unsigned short int usint;

typedef union usi
{
    unsigned char b[2];
    unsigned short int hl;
} us_int;

typedef union uli
{
    unsigned char b[4];
    unsigned long int hl;
} ul_int;

void read_map( string filename, string mapfile )
{
    FILE *file;
    file = fopen(filename.c_str(), "r");
    
    if(file == NULL)
    {
        cout << "Error opening " << filename << endl;
        exit(1);
    }
    
    fseek( file, 0x40, SEEK_SET );
    unsigned long int mesh_start;
    fread( &mesh_start, 4, 1, file );
    cout << "Mesh data at address " <<  mesh_start << endl;
    fseek( file, mesh_start, SEEK_SET );
    
    //read header...
    usint header[ 4 ];
    header[ 0 ] = 0;
    header[ 1 ] = 0;
    header[ 2 ] = 0;
    header[ 3 ] = 0;
    
    us_int h_tmp;
    
    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 0 ] = (usint) h_tmp.hl;
    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 1 ] = (usint) h_tmp.hl;
    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 2 ] = (usint) h_tmp.hl;
    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 3 ] = (usint) h_tmp.hl;
    
    
    
    
    usint N = header[ 0 ];
    usint P = header[ 1 ];
    usint Q = header[ 2 ];
    usint R = header[ 3 ];
    //cout.setf( ios::hex );
    cout << "Triangles in Map: " << N << endl;
    cout << "Quads in Map: " << P << endl;
    cout << "Untextured Triangles in Map: " << Q << endl;
    cout << "Untextured Quads in Map: " << R << endl;
    //
    // Read in textured triangles...
    //
    float triangles[ N+(P*2) ][ 3 ][ 3 ];
    for( usint x = 0; x < N; x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            for( usint z = 0; z < 3; z++ )
            {
                short int v;
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                v = (short int) v_tmp.hl;
                if( z == 1 ) v = -v;
                triangles[ x ][ y ][ z ] = (float)v / 100;
            }
        }
    }
    //
    // Read in textured quads, converting to triangles.
    //
    for( usint x = N; x < N+(P*2); )
    {
        //first triangle
        for( usint y = 0; y < 3; y++ )
        {
            for( usint z = 0; z < 3; z++ )
            {
                short int v;
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                v = (short int) v_tmp.hl;
                if( z == 1 ) v = -v;
                triangles[ x ][ y ][ z ] = (float)v / 100;
            }
        }
        x++;
        //
        // Second triangle
        //
        // first point
        triangles[ x ][ 2 ][ 0 ] = triangles[ x-1 ][ 2 ][ 0 ];
        triangles[ x ][ 2 ][ 1 ] = triangles[ x-1 ][ 2 ][ 1 ];
        triangles[ x ][ 2 ][ 2 ] = triangles[ x-1 ][ 2 ][ 2 ];
        // second point
        triangles[ x ][ 0 ][ 0 ] = triangles[ x-1 ][ 1 ][ 0 ];
        triangles[ x ][ 0 ][ 1 ] = triangles[ x-1 ][ 1 ][ 1 ];
        triangles[ x ][ 0 ][ 2 ] = triangles[ x-1 ][ 1 ][ 2 ];
        // third point
        for( usint z = 0; z < 3; z++ )
        {
            short int v;
            us_int v_tmp;
            fread( &v_tmp.b[0], 1, 1, file );
            fread( &v_tmp.b[1], 1, 1, file );
            v = (short int) v_tmp.hl;
            if( z == 1 ) v = -v;
            triangles[ x ][ 1 ][ z ] = (float)v / 100;
        }
        x++;
    }
    //
    // Untextured stuff
    //
    //
    // Read in untextured triangles...
    //
    float ut_tri[ Q+(R*2) ][ 3 ][ 3 ];
    for( usint x = 0; x < Q; x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            for( usint z = 0; z < 3; z++ )
            {
                short int v;
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                v = (short int) v_tmp.hl;
                if( z == 1 ) v = -v;
                ut_tri[ x ][ y ][ z ] = (float)v / 100;
            }
        }
    }
    //
    // Read in untextured quads, converting to triangles.
    //
    for( usint x = Q; x < Q+(R*2); )
    {
        //first triangle
        for( usint y = 0; y < 3; y++ )
        {
            for( usint z = 0; z < 3; z++ )
            {
                short int v;
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                v = (short int) v_tmp.hl;
                if( z == 1 ) v = -v;
                ut_tri[ x ][ y ][ z ] = (float)v / 100;
            }
        }
        x++;
        //
        // Second triangle
        //
        // first point
        ut_tri[ x ][ 2 ][ 0 ] = ut_tri[ x-1 ][ 2 ][ 0 ];
        ut_tri[ x ][ 2 ][ 1 ] = ut_tri[ x-1 ][ 2 ][ 1 ];
        ut_tri[ x ][ 2 ][ 2 ] = ut_tri[ x-1 ][ 2 ][ 2 ];
        // second point
        ut_tri[ x ][ 0 ][ 0 ] = ut_tri[ x-1 ][ 1 ][ 0 ];
        ut_tri[ x ][ 0 ][ 1 ] = ut_tri[ x-1 ][ 1 ][ 1 ];
        ut_tri[ x ][ 0 ][ 2 ] = ut_tri[ x-1 ][ 1 ][ 2 ];
        // third point
        for( usint z = 0; z < 3; z++ )
        {
            short int v;
            us_int v_tmp;
            fread( &v_tmp.b[0], 1, 1, file );
            fread( &v_tmp.b[1], 1, 1, file );
            v = (short int) v_tmp.hl;
            if( z == 1 ) v = -v;
            ut_tri[ x ][ 1 ][ z ] = (float)v / 100;
        }
        x++;
    }
    //
    // Read in normals...
    //
    float tnorms[ N+(P*2) ][ 3 ][ 3 ];
    for( usint x = 0; x < N; x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            for( usint z = 0; z < 3; z++ )
            {
                short int v;
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                v = (short int) v_tmp.hl;
                if( z == 1 ) v = -v;
                tnorms[ x ][ y ][ z ] = (float)v / 100;
            }
        }
    }
    //
    // Read in normals for quads, converting to triangles.
    //
    for( usint x = N; x < N+(P*2); )
    {
        //first triangle
        for( usint y = 0; y < 3; y++ )
        {
            for( usint z = 0; z < 3; z++ )
            {
                short int v;
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                v = (short int) v_tmp.hl;
                if( z == 1 ) v = -v;
                tnorms[ x ][ y ][ z ] = (float)v / 100;
            }
        }
        x++;
        //
        // Second triangle
        //
        // first point
        tnorms[ x ][ 2 ][ 0 ] = tnorms[ x-1 ][ 2 ][ 0 ];
        tnorms[ x ][ 2 ][ 1 ] = tnorms[ x-1 ][ 2 ][ 1 ];
        tnorms[ x ][ 2 ][ 2 ] = tnorms[ x-1 ][ 2 ][ 2 ];
        // second point
        tnorms[ x ][ 0 ][ 0 ] = tnorms[ x-1 ][ 1 ][ 0 ];
        tnorms[ x ][ 0 ][ 1 ] = tnorms[ x-1 ][ 1 ][ 1 ];
        tnorms[ x ][ 0 ][ 2 ] = tnorms[ x-1 ][ 1 ][ 2 ];
        // third point
        for( usint z = 0; z < 3; z++ )
        {
            short int v;
            us_int v_tmp;
            fread( &v_tmp.b[0], 1, 1, file );
            fread( &v_tmp.b[1], 1, 1, file );
            v = (short int) v_tmp.hl;
            if( z == 1 ) v = -v;
            tnorms[ x ][ 1 ][ z ] = (float)v / 100;
        }
        x++;
    }
    
    //
    // Read in Texture data...
    // 
    short int uv_coords[ N+(P*2) ][ 3 ][ 2 ];
    short int palt_page[ N+(P*2) ][ 2 ];
    for( usint x = 0; x < N; x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            short int v;
            us_int v_tmp;
            fread( &v_tmp.b[0], 1, 1, file );
            fread( &v_tmp.b[1], 1, 1, file );
            uv_coords[ x ][ y ][ 0 ] = (short int) v_tmp.b[0];
            uv_coords[ x ][ y ][ 1 ] = (short int) v_tmp.b[1];
            
            if( y == 0 )
            {
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                
                palt_page[ x ][ 0 ] = v_tmp.b[0];
            }
            if( y == 1 )
            {
                // page number
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                
                v_tmp.b[ 0 ] = v_tmp.b[ 0 ] & 3;
                
                v = (short int) v_tmp.hl;
                palt_page[ x ][ 1 ] = v *256;
            }
        }
      uv_coords[ x ][ 0 ][ 1 ] = uv_coords[ x ][ 0 ][ 1 ] + palt_page[ x ][ 1 ];
      uv_coords[ x ][ 1 ][ 1 ] = uv_coords[ x ][ 1 ][ 1 ] + palt_page[ x ][ 1 ];
      uv_coords[ x ][ 2 ][ 1 ] = uv_coords[ x ][ 2 ][ 1 ] + palt_page[ x ][ 1 ];
    }
    //
    // Read in texture for quads, converting to triangles.
    //
    for( usint x = N; x < N+(P*2); )
    {
        //First triangle
        for( usint y = 0; y < 3; y++ )
        {
            short int v;
            us_int v_tmp;
            fread( &v_tmp.b[0], 1, 1, file );
            fread( &v_tmp.b[1], 1, 1, file );
            uv_coords[ x ][ y ][ 0 ] = (short int) v_tmp.b[0];
            uv_coords[ x ][ y ][ 1 ] = (short int) v_tmp.b[1];
            
            if( y == 0 )
            {
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                
                palt_page[ x ][ 0 ] = v_tmp.b[0];
            }
            if( y == 1 )
            {
                // page number
                us_int v_tmp;
                fread( &v_tmp.b[0], 1, 1, file );
                fread( &v_tmp.b[1], 1, 1, file );
                
                v_tmp.b[ 0 ] = v_tmp.b[ 0 ] & 3;
                
                v = (short int) v_tmp.hl;
                palt_page[ x ][ 1 ] = v *256;
                //uv_coords[ x ][ y ][ 1 ] = uv_coords[ x ][ y ][ 1 ] + 
                //                           palt_page[ x ][ 1 ];
            }
        }
      uv_coords[ x ][ 0 ][ 1 ] = uv_coords[ x ][ 0 ][ 1 ] + palt_page[ x ][ 1 ];
      uv_coords[ x ][ 1 ][ 1 ] = uv_coords[ x ][ 1 ][ 1 ] + palt_page[ x ][ 1 ];
      uv_coords[ x ][ 2 ][ 1 ] = uv_coords[ x ][ 2 ][ 1 ] + palt_page[ x ][ 1 ];
        x++;
        //
        // Second triangle
        //
        palt_page[ x ][ 0 ] = palt_page[ x-1 ][0];
        short int p = palt_page[ x ][ 1 ] = palt_page[ x-1 ][1];
        
        uv_coords[ x ][ 2 ][ 0 ] = uv_coords[ x-1 ][ 2 ][ 0 ];
        uv_coords[ x ][ 2 ][ 1 ] = uv_coords[ x-1 ][ 2 ][ 1 ];

        uv_coords[ x ][ 0 ][ 0 ] = uv_coords[ x-1 ][ 1 ][ 0 ];
        uv_coords[ x ][ 0 ][ 1 ] = uv_coords[ x-1 ][ 1 ][ 1 ];
        
        us_int v_tmp;
        fread( &v_tmp.b[0], 1, 1, file );
        fread( &v_tmp.b[1], 1, 1, file );

        uv_coords[ x ][ 1 ][ 0 ] = ( short int ) v_tmp.b[0];
        uv_coords[ x ][ 1 ][ 1 ] = ( short int ) v_tmp.b[1] + p;

        x++;
    }
    fseek( file, 4 * Q + 4 * R, SEEK_CUR );
    fseek( file, 2 * N + 2 * P, SEEK_CUR );
    unsigned short int palettes[ 16 ][ 16 ][4];
    char zero = true;
    for( int x = 0; x < 16; x++ )
    {
        for( int y = 0; y < 16; y++ )
        {
            us_int v_tmp;
            fread( &v_tmp.b[0], 1, 1, file );
            fread( &v_tmp.b[1], 1, 1, file );
            unsigned char A, R, G, B;
            A = (v_tmp.b[1] & 0x80) >> 8; // one or zero
            B = (v_tmp.b[1] & 0x7c) >> 2;
            unsigned char t = (v_tmp.b[1] & 0x03) << 3;
            G = (v_tmp.b[0] & 0xe0) >> 5;
            G = G + t;
            R = (v_tmp.b[0] & 0x1F);
            palettes[ x ][ y ][0] = A*255;
            palettes[ x ][ y ][1] = R*7.96875f; // put the values into 255 color
            palettes[ x ][ y ][2] = G*7.96875f;
            palettes[ x ][ y ][3] = B*7.96875f;
            
            if( (R == G == B == 0) && (zero != false) ) zero = true;
            else zero = false;
        }
        if( zero == true )
        cout << "Palette number " << x << " is zero" << endl;
    }
    cout << "Creating and writing palettes..." << endl;
    FILE *mf, *nf;
    mf = fopen(mapfile.c_str(), "r");
    for( int px = 0; px < 16; px++ )
    {
        char texfile[50];
        sprintf( texfile, "%s_palette%u.tex", mapfile.c_str( ), px );
        nf = fopen( texfile , "w");
        for( int x = 0; x < (256*1024*4/8); x++ )
        {
            unsigned char p;
            fread( &p, 1, 1, mf );
            unsigned char p1 = (p & 0xF0) >> 4;
            unsigned char p2 = (p & 0x0F);
            //fwrite( &palettes[ 0 ][ p1 ][0], 1, 1, nf );
            fwrite( &palettes[ px ][ p1 ][1], 1, 1, nf );
            fwrite( &palettes[ px ][ p1 ][2], 1, 1, nf );
            fwrite( &palettes[ px ][ p1 ][3], 1, 1, nf );
        
            //fwrite( &palettes[ 0 ][ p2 ][0], 1, 1, nf );
            fwrite( &palettes[ px ][ p2 ][1], 1, 1, nf );
            fwrite( &palettes[ px ][ p2 ][2], 1, 1, nf );
            fwrite( &palettes[ px ][ p2 ][3], 1, 1, nf );
        }
        fclose( nf );
    }
    //
    // Convert all this stuff to OBJ format
    //
    ofstream file2;
    char outpf[50];
    sprintf( outpf, "%s.obj", filename.c_str() );
    file2.open( outpf );
    file2 << "mtllib " << filename << ".mtl" << endl;
    file2 << "o terrain" << endl;
    //Output triangles first.
    for( usint x =0; x < N+(P*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            file2 << "v ";
            file2 << triangles[ x ][ y ][ 0 ] << " ";
            file2 << triangles[ x ][ y ][ 1 ] << " ";
            file2 << triangles[ x ][ y ][ 2 ];
            file2 << endl;
        }
        //file2 << endl;
    }
    for( usint x =0; x < N+(P*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            file2 << "vn ";
            file2 << tnorms[ x ][ y ][ 0 ] << " ";
            file2 << tnorms[ x ][ y ][ 1 ] << " ";
            file2 << tnorms[ x ][ y ][ 2 ];
            file2 << endl;
        }
        //file2 << endl;
    }
    for( usint x =0; x < N+(P*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            file2 << "vt ";
            file2 << ( ((float)uv_coords[ x ][ y ][ 0 ]) / 256) << " ";
            file2 << ( ((float)uv_coords[ x ][ y ][ 1 ]) /1024) << " ";
            file2 << endl;
        }
        //file2 << endl;
    }
    file2 << "g blah" << endl;
    file2 << "usemtl blah" << endl;
    unsigned short int n = 1;
    for( usint x = 0; x < N+(P*2); x++ )
    {
        file2 << "f ";
        for( usint y = 0; y < 3; y++ )
        {
            file2 << n << "/" << n << "/" << n << " ";
            //file2 << n << " ";
            n++;
        }
        file2 << endl;
    }
    file2 << "o skirt" << endl;
    for( usint x =0; x < Q+(R*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            file2 << "v ";
            file2 << ut_tri[ x ][ y ][ 0 ] << " ";
            file2 << ut_tri[ x ][ y ][ 1 ] << " ";
            file2 << ut_tri[ x ][ y ][ 2 ];
            file2 << endl;
        }
        //file2 << endl;
    }
    for( usint x = 0; x < Q+(R*2); x++ )
    {
        file2 << "f ";
        for( usint y = 0; y < 3; y++ )
        {
            file2 << n << "//" << n << " ";
            //file2 << n << " ";
            n++;
        }
        file2 << endl;
    }
}

int main(int argc, char **argv)
{
    if( argc != 2 )
    {
        read_map( argv[1], argv[2] );
    }
    else
    {
        cout << "Gotta give map and texture names" << endl;
    }
}

