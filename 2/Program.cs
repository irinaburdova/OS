using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Collections;

namespace CheckFiles {
    class Program {
        public static void Main(string[] args) {
            string gen_filename;
            try {
                List<string> filenames = new List<string>();
                if (args.Length != 1) {
                    Console.WriteLine("Usage: CheckFiles.exe filename");
                    System.Environment.Exit(0);
                }
                gen_filename = args[0];
                if (File.Exists(gen_filename)) {
                    foreach (string line in File.ReadAllLines(gen_filename)) {
                        if (!String.IsNullOrEmpty(line)) {
                            if (File.Exists(line)) {
                                filenames.Add(line);
                            } else {
                                Console.WriteLine("Error {0}: File {1} doesn't exists", gen_filename, line);
                            }
                        }
                    }
                    List<string> all_numbers = new List<string>();
                    List<Int64> numbers = new List<Int64>();
                    foreach (string file in filenames) {
                        string number = "";
                        using (StreamReader sr = new StreamReader(file)) {
                            while (sr.Peek() >= 0) {
                                char symbl = (char)sr.Read();
                                long num;
                                if (Int64.TryParse(symbl.ToString(), out num)) {
                                    number += symbl;
                                }

                                if (!Char.IsDigit((char)sr.Peek())) {
                                    if (!String.IsNullOrEmpty(number)) {
                                        numbers.Add(Int64.Parse(number));
                                        number = "";
                                    }
                                }
                            }
                            if (!numbers.Any()) {
                                Console.WriteLine("There is no numbers in {0}", file);
                            }
                            sr.Close();
                            sr.Dispose();
                        }
                    }
                    var unique = numbers.Distinct().ToList();
                    if (unique.Any()) {
                        unique.Sort();
                        using (StreamWriter sw = new StreamWriter("out.txt")) {
                            foreach (int num in unique) {
                                sw.WriteLine(num);
                            }
                        }
                    }
                    return;
                } else {
                    Console.WriteLine("File {0} doesn't exists", args[0]);
                    return;
                }
            } catch (Exception e) {
                Console.WriteLine("Error: ");
                Console.WriteLine(e.Message);
            }
        }
    }
}
