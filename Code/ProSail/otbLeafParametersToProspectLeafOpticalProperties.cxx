/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbLeafParametersToProspectLeafOpticalProperties_cxx
#define __otbLeafParametersToProspectLeafOpticalProperties_cxx

#include "itkNumericTraits.h"

#include "otbLeafParametersToProspectLeafOpticalProperties.h"
#include <boost/math/special_functions/expint.hpp>
#include <boost/shared_ptr.hpp>
#include "otbMath.h"

//TODO check EPSILON matlab
#define EPSILON 0.0000000000000000000000001 

namespace otb
{

LeafParametersToProspectLeafOpticalProperties
::LeafParametersToProspectLeafOpticalProperties()
{
   this->ProcessObject::SetNumberOfRequiredInputs(1);
   this->ProcessObject::SetNumberOfRequiredOutputs(1);
   
   LeafOpticalPropertiesType::Pointer output = static_cast<LeafOpticalPropertiesType *>(this->MakeOutput(0).GetPointer());
}


void
LeafParametersToProspectLeafOpticalProperties
::SetInput(const LeafParametersType * object)
{
   this->itk::ProcessObject::SetNthInput(0,const_cast<LeafParametersType *>(object));
}

LeafParametersToProspectLeafOpticalProperties::LeafParametersType *
LeafParametersToProspectLeafOpticalProperties
::GetInput()
{
   if(this->GetNumberOfInputs() != 1)
   {
      //exit
      return 0;
   }
   return static_cast<LeafParametersType *>(this->itk::ProcessObject::GetInput(0));
}


LeafParametersToProspectLeafOpticalProperties::DataObjectPointer
LeafParametersToProspectLeafOpticalProperties
::MakeOutput(unsigned int)
{
   return static_cast<itk::DataObject *>(LeafOpticalPropertiesType::New().GetPointer());
}

LeafParametersToProspectLeafOpticalProperties::LeafOpticalPropertiesType *
LeafParametersToProspectLeafOpticalProperties
::GetOutput()
{
   if(this->GetNumberOfOutputs() < 1)
   {
      //exit
      return 0;
   }
   return static_cast<LeafOpticalPropertiesType *>(this->itk::ProcessObject::GetOutput(0));
}

/** Plant Leaf Reflectance and Transmittance computation from 400nm to 2500 nm*/
void
LeafParametersToProspectLeafOpticalProperties
::GenerateData()
{
   
   LeafParametersType::Pointer leafParameters = this->GetInput();
   LeafOpticalPropertiesType::Pointer output = this->GetOutput();
   
   unsigned int alpha=40;
   double lambda, n, k, trans, t12, temp, t21, r12, r21, x, y, ra, ta, r90, t90;
   double delta, beta, va, vb, vbNN, vbNNinv, vainv, s1, s2, s3, RN, TN;
   double N, Cab, Car, CBrown, Cw, Cm;
   LeafOpticalPropertiesType::VectorPairType reflVector;
   LeafOpticalPropertiesType::VectorPairType transVector;
   
   N = leafParameters->GetN();
   Cab = leafParameters->GetCab();
   Car = leafParameters->GetCar();
   CBrown = leafParameters->GetCBrown();
   Cw = leafParameters->GetCw();
   Cm = leafParameters->GetCm();
   
   int nbdata = sizeof(dataSpecP5B) / sizeof(dataSpec);
   for (int i = 0 ; i < nbdata ; i++)
   {
      lambda = dataSpecP5B[i].lambda;
      n = dataSpecP5B[i].refLeafMatInd;
      
      k = Cab*dataSpecP5B[i].chlAbsCoef+Car*dataSpecP5B[i].carAbsCoef+CBrown*dataSpecP5B[i].brownAbsCoef+Cw*dataSpecP5B[i].waterAbsCoef;
      k = k + Cm*dataSpecP5B[i].dryAbsCoef;
      k = k / N;
      if(k == itk::NumericTraits<double>::ZeroValue() ) k=EPSILON;

      trans=(1.-k)*exp(-k)+k*k*boost::math::expint(1,k);
      
      t12 = this->Tav(alpha,n);
      temp = this->Tav(90,n);
      
      
      t21 = temp/(n*n);
      r12 = 1.-t12;
      r21 = 1.-t21;
      x = t12/temp;
      y = x*(temp-1)+1-t12;
      
      ra = r12+(t12*t21*r21*(trans*trans))/(1.-r21*r21*trans*trans);
      ta = (t12*t21*trans)/(1.-r21*r21*trans*trans);
      r90 = (ra-y)/x;
      t90 = ta/x;
      
      delta = (t90*t90-r90*r90-1.)*(t90*t90-r90*r90-1.) - 4.*r90*r90;
      if(delta < 0) delta = EPSILON;
      else delta=vcl_sqrt(delta);
      
      beta = (1.+r90*r90-t90*t90-delta)/(2.*r90);
      va=(1.+r90*r90-t90*t90+delta)/(2.*r90);
      if ((beta-r90)<=0)
         vb=vcl_sqrt(beta*(va-r90)/(va*EPSILON));
      else
         vb=vcl_sqrt(beta*(va-r90)/(va*(beta-r90)));
   
      vbNN = vcl_pow(vb,N-1.);
      vbNNinv = 1./vbNN;
      vainv = 1./va;
      s1=ta*t90*(vbNN-vbNNinv);
      s2=ta*(va-vainv);
      s3=va*vbNN-vainv*vbNNinv-r90*(vbNN-vbNNinv);

      RN=ra+s1/s3;
      TN=s2/s3;


      LeafOpticalPropertiesType::PairType refl;
      LeafOpticalPropertiesType::PairType trans;
      refl.first=lambda;
      refl.second=RN;
      trans.first=lambda;
      trans.second=TN;
      reflVector.push_back(refl);
      transVector.push_back(trans);
   }
   
   std::cout<<"OUTPUT"<<std::endl;
   output->SetReflectance(reflVector);
   output->SetTransmitance(transVector);
   
}


double
LeafParametersToProspectLeafOpticalProperties
::Tav(const int theta, double ref)
{

   double theta_rad = theta*CONST_PI/180;
   double r2, rp, rm, a, k, ds, k2, rm2, res, b1, b2, b;
   double ts, tp1, tp2, tp3, tp4, tp5, tp; 

   r2=ref*ref;
   rp=r2+1;
   rm=r2-1;
   a=(ref+1)*(ref+1)/2;
   k=-(r2-1)*(r2-1)/4;
   ds=sin(theta_rad);
   
   k2=k*k;
   rm2=rm*rm;
   
   if(theta_rad==0) res=4*ref/((ref+1)*(ref+1));
   else
   {
      if(theta_rad==CONST_PI/2) b1=itk::NumericTraits<double>::ZeroValue();
      else b1=vcl_sqrt((ds*ds-rp/2)*(ds*ds-rp/2)+k);

      b2=ds*ds-rp/2;
      b=b1-b2;
      ts=(k2/(6*vcl_pow(b,3))+k/b-b/2)-(k2/(6*vcl_pow(a,3))+k/a-a/2);
      tp1=-2*r2*(b-a)/(rp*rp);
      tp2=-2*r2*rp*log(b/a)/rm2;
      tp3=r2*(1./b-1./a)/2;
      tp4=16*r2*r2*(r2*r2+1)*log((2*rp*b-rm2)/(2*rp*a-rm2))/(vcl_pow(rp,3)*rm2);
      tp5=16*vcl_pow(r2,3)*(1./(2*rp*b-rm2)-1./(2*rp*a-rm2))/vcl_pow(rp,3);
      tp=tp1+tp2+tp3+tp4+tp5;
      res=(ts+tp)/(2*ds*ds);
   }
   return res;
   
   
}

void
LeafParametersToProspectLeafOpticalProperties
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
   Superclass::PrintSelf(os,indent);

}
} // end namespace otb

#endif
