import React, { useState, useMemo } from 'react';
import { createPortal } from 'react-dom';

// --- Иконки (Встроенные SVG для обхода багов импорта) ---
const X = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><path d="M18 6 6 18"/><path d="m6 6 12 12"/></svg>);
const CalendarDays = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><rect width="18" height="18" x="3" y="4" rx="2" ry="2"/><line x1="16" x2="16" y1="2" y2="6"/><line x1="8" x2="8" y1="2" y2="6"/><line x1="3" x2="21" y1="10" y2="10"/><path d="M8 14h.01"/><path d="M12 14h.01"/><path d="M16 14h.01"/><path d="M8 18h.01"/><path d="M12 18h.01"/><path d="M16 18h.01"/></svg>);
const Clock = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/></svg>);
const Check = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><path d="M20 6 9 17l-5-5"/></svg>);
const ChevronRight = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><path d="m9 18 6-6-6-6"/></svg>);
const ChevronLeft = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><path d="m15 18-6-6 6-6"/></svg>);
const Maximize2 = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><polyline points="15 3 21 3 21 9"/><polyline points="9 21 3 21 3 15"/><line x1="21" y1="3" x2="14" y2="10"/><line x1="3" y1="21" x2="10" y2="14"/></svg>);
const Download = ({ className, strokeWidth = 2 }) => (<svg className={className} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={strokeWidth} strokeLinecap="round" strokeLinejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>);

const timeSlots =["10:00", "12:00", "14:30", "16:00", "18:00", "19:30"];

export default function CatModal({ cat, onClose, onSubmitBooking }) {
    const [currentImg, setCurrentImg] = useState(0);
    const [isFullscreen, setIsFullscreen] = useState(false);
    const [activeTab, setActiveTab] = useState('pickup');
    const[pickup, setPickup] = useState({ day: 0, time: null });
    const[dropoff, setDropoff] = useState({ day: null, time: null });
    const [isBooking, setIsBooking] = useState(false);
    const [isSuccess, setIsSuccess] = useState(false);

    // Генерируем 30 дней для календарика
    const days = useMemo(() => {
        const arr =[];
        const today = new Date();
        const dayNames =['Вс', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб'];
        const monthNames =['янв', 'фев', 'мар', 'апр', 'мая', 'июн', 'июл', 'авг', 'сен', 'окт', 'ноя', 'дек'];
        for (let i = 0; i < 30; i++) {
            const nextDate = new Date(today);
            nextDate.setDate(today.getDate() + i);
            arr.push({
                id: i,
                date: nextDate,
                dayName: i === 0 ? 'Сегодня' : i === 1 ? 'Завтра' : dayNames[nextDate.getDay()],
                dayNumber: nextDate.getDate(),
                monthName: monthNames[nextDate.getMonth()],
                isWeekend: nextDate.getDay() === 0 || nextDate.getDay() === 6
            });
        }
        return arr;
    },[]);

    const handleBackdropClick = (e) => {
        if (e.target === e.currentTarget) onClose();
    };

    const isValid = pickup.day !== null && pickup.time !== null && dropoff.day !== null && dropoff.time !== null;

    // Форматируем выбранную дату для бекенда
    const getFormattedDate = (dayId, timeIdx) => {
        const dayObj = days.find(d => d.id === dayId);
        if (!dayObj) return null;
        const d = dayObj.date;
        const yyyy = d.getFullYear();
        const mm = String(d.getMonth() + 1).padStart(2, '0');
        const dd = String(d.getDate()).padStart(2, '0');
        return `${yyyy}-${mm}-${dd} ${timeSlots[timeIdx]}:00`;
    };

    const handleBook = async () => {
        if (!isValid) return;
        setIsBooking(true);

        if (onSubmitBooking) {
            // Бронирование через API
            const startStr = getFormattedDate(pickup.day, pickup.time);
            const endStr = getFormattedDate(dropoff.day, dropoff.time);

            const success = await onSubmitBooking({ id: cat.id, start: startStr, end: endStr });
            if (success) {
                setIsBooking(false);
                setIsSuccess(true);
                setTimeout(() => { onClose(); setIsSuccess(false); }, 2000);
            } else {
                setIsBooking(false); // Ошибка вывелась через alert в каталоге
            }
        } else {
            // Если компонент вызван без API (fallback)
            setTimeout(() => {
                setIsBooking(false); setIsSuccess(true);
                setTimeout(() => { onClose(); setIsSuccess(false); }, 2000);
            }, 1200);
        }
    };

    const currentSelection = activeTab === 'pickup' ? pickup : dropoff;
    const setCurrentSelection = (updates) => {
        if (activeTab === 'pickup') {
            const newPickup = { ...pickup, ...updates };
            setPickup(newPickup);
            if (dropoff.day !== null && dropoff.day < newPickup.day) {
                setDropoff({ day: null, time: null });
            } else if (dropoff.day === newPickup.day && dropoff.time !== null && newPickup.time !== null && dropoff.time <= newPickup.time) {
                setDropoff({ ...dropoff, time: null });
            }
        } else {
            setDropoff({ ...dropoff, ...updates });
        }
    };

    const isDayDisabled = (dayId) => activeTab === 'dropoff' && pickup.day !== null && dayId < pickup.day;
    const isTimeDisabled = (timeIdx) => activeTab === 'dropoff' && pickup.day !== null && currentSelection.day === pickup.day && pickup.time !== null && timeIdx <= pickup.time;

    const nextImg = (e) => { e?.stopPropagation(); setCurrentImg((prev) => (prev + 1) % cat.images.length); };
    const prevImg = (e) => { e?.stopPropagation(); setCurrentImg((prev) => (prev - 1 + cat.images.length) % cat.images.length); };

    const handleDownloadPdf = () => {
        const blob = new Blob([`Медицинская карта питомца\nИмя: ${cat.name}\n\n(Сгенерировано автоматически)`], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a'); a.href = url; a.download = `${cat.name}_med_card.txt`;
        document.body.appendChild(a); a.click(); document.body.removeChild(a); URL.revokeObjectURL(url);
    };

    let buttonText = 'Забронировать визит';
    if (isSuccess) buttonText = 'Успешно забронировано!';
    else if (pickup.time === null) buttonText = 'Выберите время: Забрать';
    else if (dropoff.day === null || dropoff.time === null) buttonText = 'Выберите дату: Вернуть';

    // Используем Portal для правильного наложения поверх Sidebar (nav)
    return createPortal(
        <>
            {/* Темный фон */}
            <div className="fixed inset-0 z-[9999] flex items-center justify-center p-4 bg-black/40 backdrop-blur-sm animate-in fade-in duration-300 font-m3" onClick={handleBackdropClick}>

                {/* Контейнер Модального окна */}
                <div className="relative w-full max-w-lg bg-white rounded-[32px] shadow-2xl overflow-hidden flex flex-col max-h-[90vh] animate-in zoom-in-95 duration-300" onClick={e => e.stopPropagation()}>
                    <button onClick={onClose} className="absolute top-4 right-4 z-50 p-2 bg-black/30 hover:bg-black/50 text-white rounded-full backdrop-blur-md transition-colors shadow-sm"><X className="w-6 h-6" /></button>

                    <div className="overflow-y-auto no-scrollbar pb-36 relative z-10">
                        <div className="relative h-72 w-full shrink-0 group">
                            <img src={cat.images[currentImg]} alt={cat.name} className="w-full h-full object-cover transition-opacity duration-300" />

                            <div className="absolute -bottom-[1px] left-0 right-0 w-full pointer-events-none leading-[0]">
                                <svg viewBox="0 0 1440 120" preserveAspectRatio="none" className="w-full h-10 block drop-shadow-[0_-4px_12px_rgba(0,0,0,0.05)]"><path fill="#FFFFFF" d="M0,64L48,58.7C96,53,192,43,288,48C384,53,480,75,576,80C672,85,768,75,864,69.3C960,64,1056,64,1152,69.3C1248,75,1344,85,1392,90.7L1440,96L1440,120L1392,120C1344,120,1248,120,1152,120C1056,120,960,120,864,120C768,120,672,120,576,120C480,120,384,120,288,120C192,120,96,120,48,120L0,120Z"></path></svg>
                            </div>

                            {cat.images.length > 1 && (
                                <div className="absolute inset-0 flex items-center justify-between px-2 opacity-0 group-hover:opacity-100 transition-opacity">
                                    <button onClick={prevImg} className="p-1.5 bg-black/30 hover:bg-black/50 text-white rounded-full backdrop-blur-sm transition-all"><ChevronLeft className="w-5 h-5" /></button>
                                    <button onClick={nextImg} className="p-1.5 bg-black/30 hover:bg-black/50 text-white rounded-full backdrop-blur-sm transition-all"><ChevronRight className="w-5 h-5" /></button>
                                </div>
                            )}
                            <button onClick={() => setIsFullscreen(true)} className="absolute top-4 left-4 p-2 bg-black/30 hover:bg-black/50 text-white rounded-full backdrop-blur-md transition-colors shadow-sm opacity-0 group-hover:opacity-100"><Maximize2 className="w-5 h-5" /></button>

                            <div className="absolute bottom-12 left-0 right-0 flex justify-center gap-1.5 z-10">
                                {cat.images.map((_, idx) => <div key={idx} className={`h-1.5 rounded-full transition-all ${currentImg === idx ? 'w-4 bg-[#FF2B4A]' : 'w-1.5 bg-white/80'}`} />)}
                            </div>
                        </div>

                        <div className="px-6 pt-2 relative z-10 bg-white">
                            <div className="flex justify-between items-end mb-2">
                                <h2 className="text-5xl font-seymour text-[#F6828C] tracking-tight drop-shadow-sm" style={{ lineHeight: "1.1" }}>{cat.name}</h2>
                                <span className="text-lg font-bold text-gray-500 mb-1">{cat.age}</span>
                            </div>
                            <p className="text-lg text-[#FF2B4A] font-bold mb-4">{cat.breed}</p>
                            <p className="text-gray-600 font-medium leading-relaxed mb-6">{cat.description}</p>

                            <div className="flex flex-wrap gap-2 mb-8">
                                {cat.tags?.map((tag, idx) => (
                                    <span key={idx} className={`px-4 py-1.5 ${tag.bg} ${tag.color} text-sm font-bold rounded-xl`}>{tag.label}</span>
                                ))}
                            </div>

                            <div className="flex justify-between items-center mb-4">
                                <h3 className="text-xl font-black text-gray-900">Медицинская карта</h3>
                                <button onClick={handleDownloadPdf} className="flex items-center gap-1.5 px-3 py-1.5 bg-[#00C4D1]/10 text-[#00C4D1] text-sm font-bold rounded-xl hover:bg-[#00C4D1]/20 transition-colors active:scale-95">
                                    <Download className="w-4 h-4" /> PDF
                                </button>
                            </div>

                            <div className="grid grid-cols-3 gap-3 mb-8">
                                {cat.medical?.map((med, idx) => {
                                    return (
                                        <div key={idx} className={`rounded-2xl p-4 flex flex-col items-center justify-center text-center gap-2 transition-transform hover:scale-105`} style={{ backgroundColor: med.bg }}>
                                            <span className="text-sm font-bold" style={{ color: med.color }}>{med.label}</span>
                                        </div>
                                    )
                                })}
                            </div>

                            <div className="bg-[#FFF5F6] rounded-[28px] p-5 mb-4 border border-[#F6828C]/10">
                                <h3 className="text-xl font-black text-gray-900 mb-4 flex items-center gap-2"><CalendarDays className="w-5 h-5 text-[#F6828C]" /> Запланировать даты</h3>
                                <div className="flex bg-[#FFE5E8] p-1.5 rounded-2xl mb-4">
                                    <button onClick={() => setActiveTab('pickup')} className={`flex-1 flex items-center justify-center gap-2 py-2.5 rounded-xl text-sm font-bold transition-all ${activeTab === 'pickup' ? 'bg-white text-[#FF2B4A] shadow-sm' : 'text-[#E06570] hover:bg-white/50'}`}>Забрать {pickup.day !== null && pickup.time !== null && <Check className="w-4 h-4 text-[#00D26A]" />}</button>
                                    <button onClick={() => setActiveTab('dropoff')} className={`flex-1 flex items-center justify-center gap-2 py-2.5 rounded-xl text-sm font-bold transition-all ${activeTab === 'dropoff' ? 'bg-white text-[#FF2B4A] shadow-sm' : 'text-[#E06570] hover:bg-white/50'}`}>Вернуть {dropoff.day !== null && dropoff.time !== null && <Check className="w-4 h-4 text-[#00D26A]" />}</button>
                                </div>

                                <div className="flex gap-2 overflow-x-auto pb-4 no-scrollbar -mx-2 px-2">
                                    {days.map((day) => {
                                        const disabled = isDayDisabled(day.id);
                                        const isSelected = currentSelection.day === day.id;
                                        return (
                                            <button key={day.id} onClick={() => !disabled && setCurrentSelection({ day: day.id, time: null })} disabled={disabled} className={`flex flex-col items-center justify-center min-w-[4.5rem] p-3 rounded-2xl transition-all ${disabled ? 'bg-white/40 text-gray-400 cursor-not-allowed opacity-50' : isSelected ? 'bg-[#F6828C] text-white shadow-md shadow-[#F6828C]/30' : 'bg-white text-gray-600 hover:bg-[#FFE5E8] border border-gray-100'}`}>
                                                <span className="text-xs font-bold uppercase tracking-wider mb-1 opacity-80">{day.dayName}</span>
                                                <span className="text-2xl font-black leading-none">{day.dayNumber}</span>
                                                <span className="text-[10px] font-bold uppercase mt-1 opacity-70">{day.monthName}</span>
                                            </button>
                                        );
                                    })}
                                </div>

                                <div className="mt-2">
                                    <div className="flex items-center gap-2 mb-3 text-gray-600 font-bold text-sm"><Clock className="w-4 h-4" /> <span>Доступное время {activeTab === 'pickup' ? 'для встречи' : 'для возврата'}</span></div>
                                    <div className="grid grid-cols-3 gap-2">
                                        {timeSlots.map((time, idx) => {
                                            const disabled = isTimeDisabled(idx);
                                            const isSelected = currentSelection.time === idx;
                                            return (
                                                <button key={idx} onClick={() => !disabled && setCurrentSelection({ time: idx })} disabled={disabled} className={`py-2.5 rounded-xl font-bold text-sm transition-all ${disabled ? 'bg-white/40 text-gray-400 border border-gray-200/50 cursor-not-allowed opacity-50' : isSelected ? 'bg-[#FF2B4A] text-white ring-2 ring-offset-2 ring-offset-[#FFF5F6] ring-[#FF2B4A]' : 'bg-white text-gray-700 border border-gray-200 hover:bg-[#FFE5E8] hover:border-[#F6828C] hover:text-[#F6828C]'}`}>
                                                    {time}
                                                </button>
                                            );
                                        })}
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div className="absolute bottom-0 left-0 right-0 p-4 bg-gradient-to-t from-white from-60% via-white/90 to-transparent pt-16 z-50 pointer-events-none">
                        <div className="pointer-events-auto">
                            <button onClick={handleBook} disabled={!isValid || isBooking || isSuccess} className={`w-full py-4 rounded-full font-extrabold text-lg flex items-center justify-center gap-2 transition-all shadow-xl ${isSuccess ? 'bg-[#00D26A] text-white shadow-[#00D26A]/30' : isValid ? 'bg-[#F6828C] text-white hover:bg-[#FF2B4A] shadow-[#F6828C]/40 active:scale-[0.98]' : 'bg-gray-200 text-gray-400 cursor-not-allowed'}`}>
                                {isSuccess ? <Check className="w-6 h-6" /> : isBooking ? <div className="w-6 h-6 border-3 border-white border-t-transparent rounded-full animate-spin" /> : null}
                                {buttonText}
                            </button>
                        </div>
                    </div>
                </div>
            </div>

            {/* Полноэкранный просмотр изображений (тоже внутри портала) */}
            {isFullscreen && (
                <div className="fixed inset-0 z-[10000] bg-black/95 flex items-center justify-center animate-in fade-in duration-200" onClick={(e) => { e.stopPropagation(); setIsFullscreen(false); }}>
                    <button onClick={(e) => { e.stopPropagation(); setIsFullscreen(false); }} className="absolute top-4 right-4 z-50 p-2 bg-white/10 hover:bg-white/20 text-white rounded-full backdrop-blur-md transition-colors"><X className="w-6 h-6" /></button>
                    {cat.images.length > 1 && <button onClick={prevImg} className="absolute left-4 p-3 bg-white/10 hover:bg-white/20 text-white rounded-full backdrop-blur-md transition-all z-50"><ChevronLeft className="w-8 h-8" /></button>}
                    <img src={cat.images[currentImg]} alt={cat.name} className="max-w-full max-h-full object-contain pointer-events-none" />
                    {cat.images.length > 1 && <button onClick={nextImg} className="absolute right-4 p-3 bg-white/10 hover:bg-white/20 text-white rounded-full backdrop-blur-md transition-all z-50"><ChevronRight className="w-8 h-8" /></button>}
                    <div className="absolute bottom-8 left-0 right-0 flex justify-center gap-2 z-50">
                        {cat.images.map((_, idx) => <div key={idx} className={`h-2 rounded-full transition-all ${currentImg === idx ? 'w-6 bg-[#F6828C]' : 'w-2 bg-white/40'}`} />)}
                    </div>
                </div>
            )}
        </>,
        document.body
    );
}