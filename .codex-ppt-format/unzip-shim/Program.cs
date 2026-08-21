using System;
using System.IO;
using System.IO.Compression;

public static class Program
{
    public static int Main(string[] args)
    {
        if (args.Length < 2) return 2;
        using (var zip = ZipFile.OpenRead(args[1]))
        {
            if (args[0] == "-Z1")
            {
                foreach (var entry in zip.Entries) Console.WriteLine(entry.FullName);
                return 0;
            }

            if (args[0] == "-p" && args.Length >= 3)
            {
                var entry = zip.GetEntry(args[2]);
                if (entry == null) return 11;
                using (var source = entry.Open())
                using (var stdout = Console.OpenStandardOutput()) source.CopyTo(stdout);
                return 0;
            }
        }
        return 2;
    }
}
