using System.Collections.Generic;

namespace RX_DigiPrint.Comparers
{
    public class FileName_Comparer:IComparer<string>
    {
        public int Compare( string x, string y )
        {
            string x_name = System.IO.Path.GetFileNameWithoutExtension(x);
            string y_name = System.IO.Path.GetFileNameWithoutExtension(y);
            int x_val=0;
            int y_val=0;
            int i;
            
            if (x_name[0] == '[' && y_name[0]=='[') 
            {                 
                for(i=1; x_name[i]!=0 &&  x_name[i]!=']'; i++) x_val=10*x_val+x_name[i]-'0'; 
                for(i=1; y_name[i]!=0 &&  y_name[i]!=']'; i++) y_val=10*y_val+y_name[i]-'0'; 
                return x_val-y_val;
            }

            return string.Compare(x_name, y_name);
        }
    }
}
