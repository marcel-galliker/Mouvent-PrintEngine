namespace RX_DigiPrint.Services
{
    public interface IRxCrypt
    {
        string Encrypt(string clearText, string Password);
        string Decrypt(string cipherText, string Password);
    }
}
