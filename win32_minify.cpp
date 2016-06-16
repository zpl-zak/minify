// (c) 2016 ZaKlaus; All Rights Reserved

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "minify_api.h"

// TOP

struct Win32Plugin
{
    HMODULE PluginDLL;
    char * PluginExtension;
    
    minify_file * MinifyFile;
    get_extension * GetExtension;
    
    Win32Plugin * Next;
};

struct Win32App
{
    WIN32_FIND_DATA ffd;
    HANDLE hFind;
 
    char ** FileNames;
    size_t FileCount;
    
    char * OutputDir;
    char * SourceDir;
    
    Win32Plugin * Plugins;
    PlatformAPI  API;
};

void
win32_get_files_in_directory(Win32App * app, char * dir)
{
    app->hFind = FindFirstFile(dir, &app->ffd);
    //printf("FILE: %s\n", app->ffd.cFileName);
    
    size_t i = 0;
    while (FindNextFile(app->hFind, &app->ffd) != 0)
    {
        
        if (i > 0) 
        {
            app->FileNames = (char **) realloc(app->FileNames, app->FileCount+1);
            app->FileNames[i-1] = (char *) malloc(strlen(app->ffd.cFileName)+1);
            strcpy_s(app->FileNames[i-1], strlen(app->ffd.cFileName)+1, app->ffd.cFileName);
            printf("FILE: %s\n", app->ffd.cFileName);
            app->FileCount++;
        }
        
        i++;
        
    }
    
    FindClose(app->hFind);
}

char *
win32_process_file(Win32App * app, char * filename, Win32Plugin * plugin)
{
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, "%s/%s", app->SourceDir, filename);
    
    FILE * f;
    fopen_s(&f, path, "r");
    
    char * c = 0;
    size_t l = 0;
    
    if (f)
    {
        fseek(f, 0, SEEK_END);
        l = ftell(f);
        fseek(f, 0, SEEK_SET);
        c = (char *) malloc(l+1);
        
        if (c)
        {
            fread(c, 1, l, f);
        }
        c[l] = 0;
    }
    else
    {
        fprintf(stderr, "Failed to read file: %s\n", path);
        exit(-3);
    }
    
    app->API.FileContent = c;
    
    char * o = plugin->MinifyFile(&app->API);
    
    fclose(f);
    
    return o;
}

void
win32_get_path(Win32App * app)
{
    char * ptr = app->SourceDir;
    int i = 0;
    while(*(ptr++) != '/' && *(ptr) != '\\' && *(ptr) != 0)
    {
        i++;
        
        if(*ptr==0)
        {
            i = -1;
        }
    }
    
    app->SourceDir[i+1] = 0;
}

void
win32_output_file(Win32App * app, char * content, char * filename)
{
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, "%s/%s", app->OutputDir, filename);
    
    FILE * f;
    fopen_s(&f, path, "w");
    
    if (!f)
    {
        fprintf(stderr, "Failed to write to file: %s\n", path);
        exit(-3);
    }
    
    fputs(content, f);
    
    fclose(f);
}

void
win32_clean_files(Win32App * app)
{
    FindClose(app->hFind);
    app->ffd = {0};
    
    for(size_t i = 0;
        i < app->FileCount;
        i++)
    {
        free(app->FileNames[i]);
    }
    
    free(app->FileNames);
}

void
win32_clean_plugins(Win32App * app)
{
    for (Win32Plugin * Iterator = app->Plugins;
         Iterator;
         Iterator = Iterator->Next)
    {
        FreeLibrary(Iterator->PluginDLL);
        free(Iterator->PluginExtension);
        Iterator->MinifyFile = 0;
        Iterator->GetExtension = 0;
    }
}

void
win32_load_plugin(Win32App * app, char * dllname)
{
    Win32Plugin * Plugin = 0;
    if (!app->Plugins)
    {
        Plugin = app->Plugins = (Win32Plugin *) malloc(sizeof(Win32Plugin));
        Plugin->Next = NULL;
    } 
    else 
    {
        Plugin = app->Plugins->Next = (Win32Plugin *) malloc(sizeof(Win32Plugin));
        Plugin->Next = NULL;
    }
    
    Plugin->PluginDLL  = LoadLibrary(dllname);
    
    Plugin->MinifyFile = (minify_file *) GetProcAddress(Plugin->PluginDLL, "MinifyFile");
    Plugin->GetExtension = (get_extension *) GetProcAddress(Plugin->PluginDLL, "GetExtension");
    
    if (!Plugin->MinifyFile || !Plugin->GetExtension)
    {
        fprintf(stderr, "FAILED TO LOAD %s\n", dllname);
        exit(-2);
    }
    
    const char * PluginExt = Plugin->GetExtension();
    printf("Loading plugin %s with extension %s\n", dllname, PluginExt);

    Plugin->PluginExtension = (char *) malloc(strlen(PluginExt));
    strcpy_s(Plugin->PluginExtension, strlen(PluginExt)+1, PluginExt); // NOTE(zaklaus): Never forget to include space for '\0' !!!
}

char *
win32_get_file_ext(char * filename)
{
//    printf("File ext'ed %s\n", filename);
    char * ext = filename;
    
    // TODO(zaklaus): Add support for hidden files
    while(*(ext++) != '.');
    
    printf("File %s has extension %s\n", filename, ext);
    
    return ext;
}

extern "C" ALLOC_MEM(Win32_AllocMem)
{
    return malloc(memsize);
}

extern "C" FREE_MEM(Win32_FreeMem)
{
    free(memptr);
}

int
main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Not enough arguments. USAGE: %s <folder_name> <output_folder_name>\n", *argv);
        exit(-1);
    }
    
    // NOTE(zaklaus): Handle folder lookup.
    
    Win32App app = {0};

    app.API = {0};
    app.API.AllocMem = &Win32_AllocMem;
    app.API.FreeMem = &Win32_FreeMem;
    
    app.SourceDir = argv[1];
    app.OutputDir = argv[2];
    CreateDirectory(argv[2], NULL);
    
    app.Plugins = NULL;
    
    // NOTE(zaklaus): Here we can load plugins.
    win32_load_plugin(&app, "minify_css.dll");
    
    win32_get_files_in_directory(&app, argv[1]);
    
    // NOTE(zaklaus): Needs to be called AFTER we list all files in directory.
    win32_get_path(&app);
    
    //win32_clean_files(&app);
    
    for (size_t i = 0; i < app.FileCount; i++)
    {
        for (Win32Plugin * It = app.Plugins;
             It;
             It = It->Next)
        {
            if (!strcmp(win32_get_file_ext(app.FileNames[i]), It->PluginExtension))
            {
                char * output = win32_process_file(&app, app.FileNames[i],  It);
                win32_output_file(&app, output, app.FileNames[i]);
            }
        }
    }
    
//    win32_clean_files(&app);
//    win32_clean_plugins(&app);
    
    return(0);
};