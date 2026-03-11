// src/pages/home.jsx
import React from 'react';

export default function Home() {
    const shapes =[
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape"><path d="M 25 80 L 25 45 A 25 25 0 0 1 75 45 L 75 80 A 10 10 0 0 1 65 90 L 35 90 A 10 10 0 0 1 25 80 Z" fill="#FF2B4A"/></svg>,
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape-reverse"><rect x="25" y="25" width="50" height="50" rx="15" transform="skewX(-15)" fill="#7838F5"/></svg>,
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape"><g fill="#00C4D1"><rect x="35" y="15" width="30" height="70" rx="15"/><rect x="15" y="35" width="70" height="30" rx="15"/><rect x="35" y="15" width="30" height="70" rx="15" transform="rotate(45 50 50)"/><rect x="15" y="35" width="70" height="30" rx="15" transform="rotate(45 50 50)"/></g></svg>,
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape-reverse"><rect x="15" y="35" width="70" height="30" rx="15" transform="rotate(-20 50 50)" fill="#FFB300"/></svg>,
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape"><g fill="#00D26A"><circle cx="35" cy="35" r="21"/><circle cx="65" cy="35" r="21"/><circle cx="35" cy="65" r="21"/><circle cx="65" cy="65" r="21"/><rect x="30" y="30" width="40" height="40" /></g></svg>,
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape-reverse"><path d="M 50 85 C 50 85 15 55 15 35 C 15 15 35 15 45 30 C 50 35 50 35 50 35 C 50 35 50 35 55 30 C 65 15 85 15 85 35 C 85 55 50 85 50 85 Z" fill="#F9157F"/></svg>,
        <svg viewBox="0 0 100 100" className="w-48 h-48 drop-shadow-xl shrink-0 spin-shape"><path d="M 50 20 Q 60 20 65 30 L 80 60 Q 85 70 75 75 L 25 75 Q 15 70 20 60 L 35 30 Q 40 20 50 20 Z" fill="#FF6B00"/></svg>,
    ];

    const ShapeTrack = () => (
        <div className="flex items-center justify-around w-max px-8 gap-16">
            {shapes.map((shape, i) => React.cloneElement(shape, { key: i }))}
        </div>
    );

    return (
        <div className="relative w-full min-h-full rounded-[32px] bg-[#F6828C] flex flex-col justify-between overflow-hidden selection:bg-white selection:text-[#F6828C]">
            <style>{`
        @keyframes marquee-right { 0% { transform: translateX(-50%); } 100% { transform: translateX(0%); } }
        .animate-marquee { display: flex; width: max-content; animation: marquee-right 40s linear infinite; }
        @keyframes move-wave { 0% { transform: translateX(0%); } 100% { transform: translateX(-50%); } }
        .animate-wave { display: flex; width: 200%; animation: move-wave 12s linear infinite; }
        @keyframes spin-slow { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .spin-shape { animation: spin-slow 15s linear infinite; }
        .spin-shape-reverse { animation: spin-slow 18s linear infinite reverse; }
      `}</style>

            <div className="relative z-20 flex flex-col items-center justify-center px-6 pt-24 pb-8 w-full max-w-6xl mx-auto text-center flex-grow">
                <h1 className="font-spray text-7xl md:text-8xl lg:text-9xl text-white tracking-wider flex items-center justify-center drop-shadow-sm">
                    Rent'A'Purr
                </h1>
                <p className="font-m3 mt-8 text-2xl md:text-3xl lg:text-4xl text-white font-extrabold tracking-tight max-w-2xl">
                    Кошечки хорошие, лишнего не скажут.
                </p>
            </div>

            <div className="relative z-10 w-full flex flex-col">
                <div className="w-full overflow-hidden leading-[0] m-0 p-0 translate-y-[1px]">
                    <div className="animate-wave">
                        <svg viewBox="0 0 1200 120" preserveAspectRatio="none" className="w-1/2 h-10 md:h-16 lg:h-20">
                            <path d="M0,60 Q75,30 150,60 T300,60 T450,60 T600,60 T750,60 T900,60 T1050,60 T1200,60 L1200,120 L0,120 Z" fill="white" />
                        </svg>
                        <svg viewBox="0 0 1200 120" preserveAspectRatio="none" className="w-1/2 h-10 md:h-16 lg:h-20">
                            <path d="M0,60 Q75,30 150,60 T300,60 T450,60 T600,60 T750,60 T900,60 T1050,60 T1200,60 L1200,120 L0,120 Z" fill="white" />
                        </svg>
                    </div>
                </div>
                <div className="bg-white w-full pt-4 pb-16 md:pt-6 md:pb-24 overflow-hidden">
                    <div className="flex animate-marquee items-center">
                        <ShapeTrack />
                        <ShapeTrack />
                    </div>
                </div>
            </div>
        </div>
    );
}