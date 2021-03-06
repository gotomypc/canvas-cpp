/**
 * Copyright (c) 2010 Daniel Wiberg
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Script.h"
#include <iostream>
#include <fstream>

namespace canvas
{
   Script::Script(v8::Handle<v8::Context> context)
   {
      m_context = v8::Persistent<v8::Context>::New(context);
   }
   
   Script::~Script()
   {
   }
   
   v8::Local<v8::Value> Script::getObject(std::string const& name)
   {
      v8::HandleScope scope;
      v8::Context::Scope contextScope(m_context);

      return scope.Close(m_context->Global()->Get(v8::String::New(name.c_str())));
   }
   
   bool Script::load(std::string const& filename)
   {
      std::vector<char> data;
      
      // Read file
      std::ifstream inf(filename.c_str(), std::ios::binary);
      if (!inf.good())
      {
         inf.close();
         return false;
      }
      
      char ch;
      while (inf.get(ch))
         data.push_back(ch);
         
      inf.close();
      
      data.push_back('\0');
      
      return runString(std::string(&data[0]), filename);
   }
   
   bool Script::load(std::vector<std::string> const& files)
   {
      std::vector<char> data;
      
      // Load all the files
      for (std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
      {
         std::string filename = *it;
      
         // Read file
         std::ifstream inf(filename.c_str(), std::ios::binary);
         if (inf.good())
         {
            // Read file
            char ch;
            while (inf.get(ch))
               data.push_back(ch);
         }
         else
            std::cerr << "Could not read file '" <<  filename << "'" << std::endl;
         
         // Insert a newline between each file
         data.push_back('\n');
         inf.close();
      }
      
      data.push_back('\0');
      return runString(std::string(&data[0]), "");
   }
      
      
   bool Script::runString(std::string const& code, std::string const& filename)
   {
      v8::HandleScope scope;
      v8::Context::Scope contextScope(m_context);
      v8::TryCatch tryCatch;
      
      // Create a v8 string to hold the source
      v8::Handle<v8::String> source = v8::String::New(code.c_str(), code.size());
      
      // Compile the script
      v8::Handle<v8::Script> script = v8::Script::Compile(source, v8::String::New(filename.c_str()));
      if (script.IsEmpty())
      {
         v8::String::Utf8Value error(tryCatch.Exception());
         v8::Handle<v8::Message> message = tryCatch.Message();
         
         std::cerr << "+----" << std::endl;
         std::cerr << "| Error in " << filename.c_str() << ":" << *error << std::endl;
         std::cerr << "| (" << message->GetLineNumber() << "):" << *v8::String::Utf8Value(message->GetSourceLine()) << std::endl;
         std::cerr << "+----" << std::endl;
         return false;
      }
      
      // Finally we run the script
      v8::Handle<v8::Value> result = script->Run();
      if (result.IsEmpty())
      {
         v8::String::Utf8Value error(tryCatch.Exception());
         std::cerr << "Script Error: " << *error << std::endl;
         return false;
      }
         
      return true;
   }

}

