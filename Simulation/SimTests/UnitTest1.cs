using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Simulation;

namespace SimTests
{
    [TestClass]
    public class UnitTest1
    {
        [TestMethod]
        public void SimTest1()
        {
            Console.WriteLine("Modelling test, instantaneous freqs");
            var o = new OCXO();
            o.SetDAC(1);
            Console.WriteLine($"DAC =  {o.GetDAC()} Current = {o.Current} Target={o.Target}");
            o.SetDAC(2500);
            Console.WriteLine($"DAC =  {o.GetDAC()} Current = {o.Current} Target={o.Target}");
            o.SetDAC(4095);
            Console.WriteLine($"DAC =  {o.GetDAC()} Current = {o.Current} Target={o.Target}");

        }
    }
}
