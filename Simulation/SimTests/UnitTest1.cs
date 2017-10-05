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
            Console.WriteLine($"DAC =  {o.GetDAC()} Current = {o.Current} Target={o.Target} Count over 2s = {o.GetCount(2000)}");
            o.SetDAC(2500);
            Console.WriteLine($"DAC =  {o.GetDAC()} Current = {o.Current} Target={o.Target} Count over 2s = {o.GetCount(2000)}");
            o.SetDAC(4095);
            Console.WriteLine($"DAC =  {o.GetDAC()} Current = {o.Current} Target={o.Target} Count over 2s = {o.GetCount(2000)}");



        }

        [TestMethod]
        public void GPSCountTest()
        {
            Console.WriteLine("GPS test, instantaneous freqs");
            var o = new GPSSource();

            // 1000 ms = 1 sec, at 10Mhz, thats 10Mil counts
            Assert.IsTrue(o.GetCount(1000)  ==10000000);
            Assert.IsTrue(o.GetCount(10000) ==100000000);

        }

        [TestMethod]
        public void BasicColdStart()
        {
            var g = new GPSSource();
            var o = new OCXO();

           var a = g.GetCount(1000000);
            var b = o.GetCount(1000000);
            Console.WriteLine($"Count DIFF over 1000 seconds = { a-b}");
        }

    }
}
