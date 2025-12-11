#include "selfutil.h"
#include "compat/elf.h"
#include <cstdio>
#include <vector>
#include <string>

using namespace std;

// Global flags (tu peux aussi les mettre dans SelfUtil si tu veux)
string input_file_path;
string output_file_path;
bool dry_run = false;
bool verbose = false;
bool overwrite = false;
bool align_size = false;
bool patch_first_segment_duplicate = true;
bool patch_version_segment = true;

// Usage helper
void print_usage()
{
    printf("Usage: selfutil --input <file> [--output <file>] [--dry-run] [--verbose] [--overwrite] [--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

// ----------------------- SelfUtil member functions ------------------------

SelfUtil::SelfUtil(const string& filePath) : filePath(filePath)
{
}

// Load function
bool SelfUtil::Load(const string& filePath)
{
    // Ici tu dois implémenter ton code pour charger le fichier SELF
    return true;
}

// SaveToELF function
bool SelfUtil::SaveToELF(const string& outFile)
{
    // Ici tu dois implémenter ton code pour sauvegarder en ELF
    return true;
}

// ----------------------- Main function ------------------------

int main(int argc, char* argv[])
{
    vector<string> args;

    if (argc < 2)
    {
        print_usage();
        return 0;
    }

    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);

    bool error_found = false;

    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i] == "--input" && i + 1 < args.size())
        {
            input_file_path = args[i + 1];
            i++;
        }
        else if (args[i] == "--output" && i + 1 < args.size())
        {
            output_file_path = args[i + 1];
            i++;
        }
        else if (args[i] == "--dry-run")
            dry_run = true;
        else if (args[i] == "--verbose")
            verbose = true;
        else if (args[i] == "--overwrite")
            overwrite = true;
        else if (args[i] == "--align-size")
            align_size = true;
        else if (args[i] == "--not-patch-first-segment-duplicate")
            patch_first_segment_duplicate = false;
        else if (args[i] == "--not-patch-version-segment")
            patch_version_segment = false;
        else
            error_found = true;
    }

    if (error_found || input_file_path.empty())
    {
        print_usage();
        return 0;
    }

    string fixed_output_file_path = output_file_path.empty() ? input_file_path : output_file_path;

    if (!overwrite && output_file_path.empty())
    {
        size_t dot_pos = input_file_path.rfind('.');
        if (dot_pos != string::npos)
            fixed_output_file_path = input_file_path.substr(0, dot_pos) + ".elf";
    }

    // Affichage des infos
    printf(
        "Input File Name: %s\n"
        "Output File Name: %s\n"
        "Dry Run: %s\n"
        "Verbose: %s\n"
        "Overwrite: %s\n"
        "Align Size: %s\n"
        "Patch First Segment Duplicate: %s\n"
        "Patch Version Segment: %s\n",
        input_file_path.c_str(),
        fixed_output_file_path.c_str(),
        dry_run ? "True" : "False",
        verbose ? "True" : "False",
        overwrite ? "True" : "False",
        align_size ? "True" : "False",
        patch_first_segment_duplicate ? "True" : "False",
        patch_version_segment ? "True" : "False"
    );

    // Création de l'objet avec constructeur correct
    SelfUtil util(input_file_path);

    if (!util.Load(input_file_path))
    {
        printf("Error, failed to load SELF file!\n");
        return 1;
    }

    if (!util.SaveToELF(fixed_output_file_path))
    {
        printf("Error, Save to ELF failed!\n");
        return 1;
    }

    return 0;
}
