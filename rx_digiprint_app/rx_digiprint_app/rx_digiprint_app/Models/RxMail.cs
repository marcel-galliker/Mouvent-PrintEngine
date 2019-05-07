using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DigiPrint.Models
{
    public class RxMail
    {
        public string To;
        public string Subject;
        public string Text;
        public string AttachmentContent;
    }
    public interface IRxMailServer
    {
        void Send(RxMail mail);
    };
}
