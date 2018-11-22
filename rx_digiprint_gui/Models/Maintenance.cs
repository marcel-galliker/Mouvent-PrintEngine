using RX_DigiPrint.Helpers;
using SmartXLS;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class Maintenance
    {
        private RxWorkBook m_WorkBook;

        //--- CreateLog -------------------------------------
        public bool CreateLog(string filepath)
        {
            m_WorkBook = new RxWorkBook();
            m_WorkBook.FileName = filepath;
            m_WorkBook.AddSheet("Print System");    RxGlobals.PrintSystem.CreateLog(m_WorkBook);
            m_WorkBook.AddSheet("Mini Label");      RxGlobals.Plc.LogInfo(m_WorkBook);
            m_WorkBook.AddSheet("Rexroth Log");     RxGlobals.Plc.LogErrors(m_WorkBook);
            m_WorkBook.AddSheet("Network");         RxGlobals.Network.CreateLog(m_WorkBook);
            m_WorkBook.AddSheet("Log");             RxGlobals.Log.CreateLog(m_WorkBook);

            m_WorkBook.SizeColumns();
            return m_WorkBook.write(filepath);
        }

    }
}
