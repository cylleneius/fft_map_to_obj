// #include <iostream>
// #include <fstream>
// #include <string>
// #include <vector>
// #include <map>
// #include <cstdlib>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

// using namespace std;

#define PNG_DEBUG 3
#include <png.h>

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

// void abort_(const char * s, ...)
// {
//         va_list args;
//         va_start(args, s);
//         vfprintf(stderr, s, args);
//         fprintf(stderr, "\n");
//         va_end(args);
//         abort();
// }

int check_mesh( char *filename )
{
    FILE *file;
    ul_int mesh_start;

    file = fopen(filename, "r");
    
    if(file == NULL)
    {
        printf( "Error opening %s\n", filename );
        return -1;
    }

    fseek( file, 0x40, SEEK_SET );

    fread( &mesh_start.b, 4, 1, file );

    if( mesh_start.b[0] != 0xc4 )
    {
        fclose( file );
        return -1;
    }
    return 0;
}

int convert( char *filename, char *mapname )
{
    printf( "Converting %s with %s\n", filename, mapname );
    if( check_mesh( filename ) != 0 )
    {
        printf( "Not a mesh file.\n" );
        return -1;
    }

    FILE *file;
    file = fopen(filename, "r");

    unsigned long int meshstart;

    fseek( file, 0x40, SEEK_SET );
    unsigned long int mesh_start;
    fread( &mesh_start, 4, 1, file );

    if( mesh_start != 196 )
    {
        printf( "Not a mesh file.\n" );
        fclose( file );
        return -1;
    }

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
  //  printf( "%i %i\n", h_tmp.b[0], h_tmp.hl );

    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 1 ] = (usint) h_tmp.hl;
  //  printf( "%i %i\n", h_tmp.b[0], h_tmp.hl );

    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 2 ] = (usint) h_tmp.hl;
  //  printf( "%i %i\n", h_tmp.b[0], h_tmp.hl );

    fread( &h_tmp.b[0], 1, 1, file );
    fread( &h_tmp.b[1], 1, 1, file );
    header[ 3 ] = (usint) h_tmp.hl;
  //  printf( "%i %i\n", h_tmp.b[0], h_tmp.hl );

    usint N = header[ 0 ];
    usint P = header[ 1 ];
    usint Q = header[ 2 ];
    usint R = header[ 3 ];

    printf( "%i\n", h_tmp.hl );
    if( N == 0 && P == 0 && Q == 0 && R == 0 )
    {
        printf( "Not a mesh file.\n" );
        fclose( file );
        return -1;
    }

    printf( "Triangles in Map: %i\n", N );
    printf( "Quads in Map: %i\n", P );
    printf( "Untextured Triangles in Map: %i\n", Q );
    printf( "Untextured Quads in Map: %i\n", R );
    
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

    // char zero = true;
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
        }
    }

    fclose( file );

    printf( "Converting %s with %s\n", filename, mapname );

    FILE *mf; 
    mf = fopen(mapname, "r");

    // png_byte color_type;
    // png_byte bit_depth;

    char texfile[50];

    // for( int px = 0; px < 16; px++ )
    // {
    int px = 0;
    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep * row_pointers;

    row_pointers = (png_bytep*)malloc( sizeof(png_bytep) * 1024);
    for (int y=0; y < 1024; y++)
        row_pointers[y] = (png_byte*) malloc(sizeof(png_bytep) * 256 * 4);
    
    sprintf( texfile, "%s_tex_%i.png", mapname, px );
    FILE *nf;
    nf = fopen( texfile , "w");

        for( int yy = 1023; yy > 0; yy-- )
        {
            int r = 0;
            for( int xx = 0; xx < 256*4; xx+=8 )
            {
                    unsigned char p;
                    fread( &p, 1, 1, mf );
                    unsigned char p1 = (p & 0xF0) >> 4;
                    unsigned char p2 = (p & 0x0F);
                    
                    row_pointers[ yy ][ r+0 ] = (unsigned char)palettes[ px ][ p1 ][1];
                    row_pointers[ yy ][ r+1 ] = (unsigned char)palettes[ px ][ p1 ][2];
                    row_pointers[ yy ][ r+2 ] = (unsigned char)palettes[ px ][ p1 ][3];
                    
                    row_pointers[ yy ][ r+3 ] = (unsigned char)palettes[ px ][ p2 ][1];
                    row_pointers[ yy ][ r+4 ] = (unsigned char)palettes[ px ][ p2 ][2];
                    row_pointers[ yy ][ r+5 ] = (unsigned char)palettes[ px ][ p2 ][3];
                    
                    r+=6;
            }
        }

        if (!nf) exit(-1);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr) exit(-1);

    info_ptr = png_create_info_struct(png_ptr);

        if (!info_ptr) exit(-1);

        if (setjmp(png_jmpbuf(png_ptr))) exit(-1);

    png_init_io(png_ptr, nf);

        if (setjmp(png_jmpbuf(png_ptr))) exit(-1);

    png_set_IHDR(png_ptr, info_ptr, 256, 1024, 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

        if (setjmp(png_jmpbuf(png_ptr))) exit(-1);

    png_write_image(png_ptr, row_pointers);

        if (setjmp(png_jmpbuf(png_ptr))) exit(-1);

    png_write_end(png_ptr, NULL);


    for(int y=0; y < 1024; y++ ) free( row_pointers[ y ] );
    free( row_pointers );

    fclose( nf );

    //
    // Convert all this stuff to OBJ format
    //
    char mtlfilename[50];
    sprintf( mtlfilename, "%s_%s.mtl", filename, mapname );

    FILE *file2;
    char outpf[80];
    sprintf( outpf, "%s_%s.obj", filename, mapname );
    file2 = fopen( outpf, "w" );

    sprintf( outpf, "mtllib %s\n", mtlfilename );
    fwrite( outpf, 1, strlen( outpf ), file2 );
    // fwrite( "mtllib ", 1, 7, file2 );
    // fwrite( filename, 1, strlen(filename), file2 );
    // fwrite( ".mtl", 1, 4, file2);
    sprintf( outpf, "o terrain\n" );
    fwrite( outpf, 1, strlen( outpf ), file2 );
    // file2 << "o terrain" << endl;
    //Output triangles first.
    for( usint x =0; x < N+(P*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            fwrite( "v ", 1, 2, file2 );
            sprintf( outpf, "%f %f %f\n", triangles[ x ][ y ][ 0 ],
                                          triangles[ x ][ y ][ 1 ],
                                          triangles[ x ][ y ][ 2 ] );
            fwrite( outpf, 1, strlen( outpf ), file2 );
        }
        //file2 << endl;
    }
    for( usint x =0; x < N+(P*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            fwrite( "vn ", 1, 3, file2 );
            // file2 << "vn ";
            sprintf( outpf, "%f %f %f\n", tnorms[ x ][ y ][ 0 ],
                                          tnorms[ x ][ y ][ 1 ],
                                          tnorms[ x ][ y ][ 2 ] );
            fwrite( outpf, 1, strlen( outpf ), file2 );
        }
        //file2 << endl;
    }
    for( usint x =0; x < N+(P*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            fwrite( "vt ", 1, 3, file2 );
            // file2 << "vt ";
            sprintf( outpf, "%f %f \n", ((float)uv_coords[ x ][ y ][ 0 ]/ 256),
                                        ((float)uv_coords[ x ][ y ][ 1 ]/1024));
            fwrite( outpf, 1, strlen( outpf ), file2 );
        }
        //file2 << endl;
    }
    fwrite( "g tex\n", 1, 6, file2 );
    // file2 << "g tex" << endl;
    fwrite( "usemtl blah\n", 1, 12, file2 );
    // file2 << "usemtl blah" << endl;
    unsigned short int n = 1;
    for( usint x = 0; x < N+(P*2); x++ )
    {
        fwrite( "f ", 1, 2, file2 );
        // file2 << "f ";
        for( usint y = 0; y < 3; y++ )
        {
            sprintf( outpf, "%d/%d/%d ", n, n, n );
            fwrite( outpf, 1, strlen( outpf ), file2 );
            // file2 << n << "/" << n << "/" << n << " ";
            //file2 << n << " ";
            n++;
        }
        // file2 << endl;
        fwrite( "\n", 1, 1, file2 );
    }
    fwrite( "o skirt\n", 1, 8, file2);// << endl;
    for( usint x =0; x < Q+(R*2); x++ )
    {
        for( usint y = 0; y < 3; y++ )
        {
            sprintf( outpf, "v %f %f %f\n", ut_tri[ x ][ y ][ 0 ],
                                            ut_tri[ x ][ y ][ 1 ],
                                            ut_tri[ x ][ y ][ 2 ] );
            fwrite( outpf, 1, strlen(outpf), file2 );
        }
        //file2 << endl;
    }
    for( usint x = 0; x < Q+(R*2); x++ )
    {
        fwrite( "f ", 1, 2, file2 );
        for( usint y = 0; y < 3; y++ )
        {
            sprintf( outpf, "%d//%d ", n, n );
            fwrite( outpf, 1, strlen(outpf), file2 );
            //file2 << n << " ";
            n++;
        }
        fwrite( "\n", 1, 1, file2 );
    }
    fclose( file2 );

    FILE *mtlfile;
    mtlfile = fopen( mtlfilename, "w" );
    sprintf( outpf, "newmtl blah\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "Ns 100.000\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "d 1.00000\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "illum 2\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "Kd 1.00000 1.00000 1.00000\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "Ka 1.00000 1.00000 1.00000\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "Ks 1.00000 1.00000 1.00000\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "Ke 1.00000 1.00000 1.00000\n" );
    fwrite( outpf, 1, strlen(outpf), mtlfile );

    sprintf( outpf, "map_Kd %s\n", texfile );
    fwrite( outpf, 1, strlen(outpf), mtlfile );
    fclose( mtlfile );
}

int walkdir(const char *path) 
{
    if( path == NULL ) return -1;
    int path_size;
    char *filename;
    char *curname;

    char **mnames;
    int num_mnames;

    char **tnames;
    int num_tnames;

    struct dirent *entry;
    DIR *dp;

    path_size = sizeof( char ) * strlen( path );
    filename = (char*)malloc( path_size + 3 ); //+".xx"
    strcpy( filename, path );

    mnames = (char**)malloc( sizeof( char* ) * 1 );
    num_mnames = 0;

    tnames = (char**)malloc( sizeof( char* ) * 1 );
    num_tnames = 0;
    
    

    dp = opendir(".");
    if (dp == NULL) 
    {
        perror("opendir");
        return -1;
    }

    while( (entry = readdir(dp)) )
    {
        if( strncmp( entry->d_name, path, path_size ) == 0 )
        {
            int i = strlen(entry->d_name);

            if( strncmp( entry->d_name + (i-3), "obj", 3 ) == 0 ) continue;
            if( strncmp( entry->d_name + (i-3), "gns", 3 ) == 0 ) continue;
            
            if( strncmp( entry->d_name + (i-3), "mtl", 3 ) == 0 ) continue;
            if( strncmp( entry->d_name + (i-3), "png", 3 ) == 0 ) continue;

            if( check_mesh( entry->d_name ) == 0 )
            {
                num_mnames++;
                mnames = (char**)realloc( mnames, sizeof( char* )*num_mnames );
                mnames[ num_mnames-1 ] = (char*)malloc( sizeof( char ) * strlen( entry->d_name ) + 1 );
                strncpy( mnames[ num_mnames-1 ], entry->d_name, strlen( entry->d_name ) +1 );
            }
            else
            {
                num_tnames++;
                tnames = (char**)realloc( tnames, sizeof( char* )*num_tnames );
                tnames[ num_tnames-1 ] = (char*)malloc( sizeof( char ) * strlen( entry->d_name ) +1 );
                strncpy( tnames[ num_tnames-1 ], entry->d_name, strlen( entry->d_name ) +1 );
            }
        }
    }

    closedir(dp);
    printf( "%i meshes found\n", num_mnames );
    printf( "%i possible textures\n", num_tnames );
    for( int i=0; i < num_mnames; i++ )
    {
        for( int j=0; j < num_tnames; j++ )
        {
            printf( "%s > %s\n", mnames[ i ], tnames[ j ] );
            convert( mnames[ i ], tnames[ j ] );
        }
    }
    return 0;
}

int main(int argc, char **argv)
{    
    walkdir( argv[1] );
}

