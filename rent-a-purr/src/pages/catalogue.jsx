import React, { useState, useMemo, useEffect } from 'react';
import { Search, Activity, Check, PawPrint } from 'lucide-react';
import CatModal from '../components/card';
// ИМПОРТИРУЕМ getImageUrl из нашего API:
import { api, getImageUrl } from '../api';

export default function Catalogue() {
    const[cats, setCats] = useState([]);
    const [loading, setLoading] = useState(true);

    const[selectedTags, setSelectedTags] = useState([]);
    const [selectedMedical, setSelectedMedical] = useState([]);
    const [filterRange, setFilterRange] = useState({ start: null, end: null });
    const[selectedCat, setSelectedCat] = useState(null);

    useEffect(() => {
        api.getCats()
            .then(res => {
                setCats(res.cats ||[]);
                setLoading(false);
            })
            .catch(err => {
                console.error("Ошибка загрузки котов:", err);
                setLoading(false);
            });
    },[]);

    const ALL_TAGS = Array.from(new Set(cats.flatMap(cat => cat.tags ||[])));

    const ALL_MEDICAL_OBJS =[];
    const seenMedical = new Set();
    cats.forEach(cat => {
        (cat.medical ||[]).forEach(m => {
            if (!seenMedical.has(m.label)) {
                seenMedical.add(m.label);
                ALL_MEDICAL_OBJS.push(m);
            }
        });
    });

    const toggleTag = (tag) => setSelectedTags(prev => prev.includes(tag) ? prev.filter(t => t !== tag) : [...prev, tag]);
    const toggleMedical = (medLabel) => setSelectedMedical(prev => prev.includes(medLabel) ? prev.filter(m => m !== medLabel) : [...prev, medLabel]);

    const filteredAndSortedCats = useMemo(() => {
        let result = [...cats];

        if (selectedTags.length > 0) {
            result = result.filter(cat => selectedTags.every(tag => (cat.tags ||[]).includes(tag)));
        }

        if (selectedMedical.length > 0) {
            result = result.filter(cat => selectedMedical.every(medLabel => (cat.medical ||[]).some(m => m.label === medLabel)));
        }

        if (filterRange.start && filterRange.end) {
            const reqStart = filterRange.start.getTime();
            const reqEnd = filterRange.end.getTime();
            if (reqStart < reqEnd) {
                result = result.filter(cat => !(cat.bookings ||[]).some(busy => {
                    return reqStart < new Date(busy[2]).getTime() && reqEnd > new Date(busy[1]).getTime();
                }));
            }
        }
        return result;
    },[cats, selectedTags, selectedMedical, filterRange]);

    const handleBookCat = async (bookingData) => {
        try {
            await api.addBooking(bookingData);
            const res = await api.getCats();
            setCats(res.cats ||[]);
            return true;
        } catch (e) {
            alert(e.message);
            return false;
        }
    };

    if (loading) {
        return <div className="h-full flex items-center justify-center font-bold text-slate-400">Загрузка котиков...</div>;
    }

    return (
        <div className="min-h-full bg-white text-slate-900 font-m3 pb-12 relative overflow-hidden">
            <div className="w-full px-4 sm:px-6 lg:px-8 pt-6 pb-2 md:pt-8 max-w-[1600px] mx-auto">
                <div className="flex w-full h-20 sm:h-24 md:h-32 lg:h-40 gap-2 sm:gap-3 md:gap-5">
                    <div className="w-5 sm:w-8 md:w-10 lg:w-14 bg-[#F6828C] rounded-full shrink-0 shadow-sm"></div>
                    <div className="flex-1 bg-[#F6828C] rounded-full flex items-center justify-center px-4 sm:px-8 shadow-sm overflow-hidden relative">
                        <h1 className="relative font-seymour text-2xl sm:text-4xl md:text-5xl lg:text-[4.5rem] text-white tracking-wider drop-shadow-sm text-center leading-none mt-1 md:mt-2">
                            Пушистики
                        </h1>
                    </div>
                    <div className="w-12 sm:w-20 md:w-28 lg:w-40 bg-[#F6828C] rounded-full shrink-0 shadow-sm"></div>
                </div>
            </div>

            <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8 relative z-10">
                <section className="bg-white rounded-[32px] p-6 sm:p-8 shadow-[0_8px_30px_rgb(0,0,0,0.04)] border-2 border-slate-100 mb-10">
                    <div className="flex flex-col xl:flex-row gap-8 justify-between">
                        <div className="flex-1 space-y-8">
                            {ALL_TAGS.length > 0 && (
                                <div>
                                    <h3 className="text-sm font-extrabold text-slate-400 uppercase tracking-widest mb-4 flex items-center gap-2">
                                        <Search size={18} className="text-[#00C4D1]" /> Характер и навыки
                                    </h3>
                                    <div className="flex flex-wrap gap-2.5">
                                        {ALL_TAGS.map(tag => (
                                            <button key={tag} onClick={() => toggleTag(tag)} className={`flex items-center gap-2 px-5 py-2.5 rounded-2xl text-sm font-bold transition-all border-2 ${selectedTags.includes(tag) ? 'bg-[#00C4D1]/10 text-[#00A8B3] border-[#00C4D1]/30' : 'bg-white text-slate-600 border-slate-100 hover:border-slate-300'}`}>
                                                {selectedTags.includes(tag) && <Check size={16} strokeWidth={3} />} {tag}
                                            </button>
                                        ))}
                                    </div>
                                </div>
                            )}

                            {ALL_MEDICAL_OBJS.length > 0 && (
                                <div>
                                    <h3 className="text-sm font-extrabold text-slate-400 uppercase tracking-widest mb-4 flex items-center gap-2">
                                        <Activity size={18} className="text-[#FF2B4A]" /> Здоровье
                                    </h3>
                                    <div className="flex flex-wrap gap-2.5">
                                        {ALL_MEDICAL_OBJS.map(med => {
                                            const isSelected = selectedMedical.includes(med.label);
                                            return (
                                                <button key={med.label} onClick={() => toggleMedical(med.label)} style={{ backgroundColor: isSelected ? med.bg : '#ffffff', color: isSelected ? med.color : '#475569', borderColor: isSelected ? med.bg : '#f1f5f9' }} className={`flex items-center gap-2 px-5 py-2.5 rounded-2xl text-sm font-extrabold transition-all border-2`}>
                                                    {med.label} {isSelected && <Check size={16} strokeWidth={4} className="ml-1 opacity-80" />}
                                                </button>
                                            );
                                        })}
                                    </div>
                                </div>
                            )}
                        </div>
                    </div>
                </section>

                <div className="mb-6 px-2 text-slate-500 font-extrabold text-lg flex items-center gap-2">
                    Найдено котиков: <span className="text-[#FFB300] text-2xl">{filteredAndSortedCats.length}</span>
                </div>

                {filteredAndSortedCats.length > 0 ? (
                    <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
                        {filteredAndSortedCats.map(cat => <CatCard key={cat.id} cat={cat} onBook={() => setSelectedCat(cat)} />)}
                    </div>
                ) : (
                    <div className="bg-slate-50 rounded-[32px] p-12 text-center border-2 border-dashed border-slate-200 relative overflow-hidden mt-8">
                        <PawPrint size={64} className="text-slate-300 mx-auto mb-6 drop-shadow-sm" />
                        <h3 className="text-2xl font-extrabold text-slate-800 mb-2">Никого не найдено</h3>
                        <button onClick={() => { setSelectedTags([]); setSelectedMedical([]); setFilterRange({start: null, end: null}); }} className="mt-4 py-4 px-8 rounded-full bg-[#F6828C] hover:bg-[#E56873] text-white font-extrabold text-lg shadow-lg transition-all active:scale-[0.98]">
                            Сбросить фильтры
                        </button>
                    </div>
                )}
            </main>

            {selectedCat && (
                <CatModal
                    cat={{
                        ...selectedCat,
                        images: Array.isArray(selectedCat.filenames) && selectedCat.filenames.length > 0
                            ? selectedCat.filenames.map(img => getImageUrl(img))
                            : [getImageUrl(selectedCat.filenames)],
                        tags: (selectedCat.tags ||[]).map(t => ({ label: t, color: "text-[#7838F5]", bg: "bg-[#7838F5]/10" }))
                    }}
                    onClose={() => setSelectedCat(null)}
                    onSubmitBooking={handleBookCat}
                />
            )}
        </div>
    );
}

function CatCard({ cat, onBook }) {
    // Вызов импортированной функции
    const coverImage = getImageUrl(cat?.filenames);

    return (
        <article className="bg-white rounded-[32px] p-4 sm:p-5 shadow-[0_4px_20px_rgb(0,0,0,0.03)] border-2 border-slate-100 hover:border-[#F6828C]/30 transition-all flex flex-col h-full group">

            <div className="relative w-full h-56 flex-shrink-0 mb-5">
                <img
                    src={coverImage}
                    alt={cat.name || 'Кот'}
                    className="w-full h-full object-cover rounded-[24px]"
                    onError={(e) => { e.target.src = 'https://via.placeholder.com/600x400?text=Файл+не+найден' }}
                />
                {cat.age && (
                    <div className="absolute top-3 right-3 bg-white/90 backdrop-blur-sm px-3 py-1.5 rounded-full text-xs font-extrabold text-slate-800 shadow-sm border border-slate-100">
                        {cat.age}
                    </div>
                )}
            </div>

            <div className="flex flex-col flex-grow">
                <div className="mb-2">
                    <h3 className="text-2xl font-extrabold text-slate-800 tracking-tight">{cat.name || 'Безымянный'}</h3>
                    <p className="text-slate-500 font-bold">{cat.breed || 'Неизвестная порода'}</p>
                </div>

                <p className="text-slate-600 font-medium text-sm mb-4 line-clamp-2">
                    {cat.description || 'Описание отсутствует.'}
                </p>

                <div className="flex flex-wrap gap-2 mb-4">
                    {(cat.tags ||[]).map(tag => (
                        <span key={tag} className="px-3 py-1.5 bg-slate-50 text-slate-600 text-[11px] uppercase tracking-wider font-extrabold rounded-xl border border-slate-100">
              {tag}
            </span>
                    ))}
                </div>

                <div className="flex flex-wrap gap-2.5 mb-6 mt-auto">
                    {(cat.medical ||[]).map(med => (
                        <div key={med.label} className="flex items-center gap-1.5 px-3 py-1.5 rounded-xl text-xs font-extrabold shadow-sm" style={{ backgroundColor: med.bg || '#eee', color: med.color || '#333' }}>
                            {med.label}
                        </div>
                    ))}
                </div>

                <button onClick={onBook} className="w-full py-4 bg-[#F6828C] text-white font-extrabold text-lg rounded-full hover:bg-[#E56873] active:scale-[0.98] shadow-lg shadow-[#F6828C]/30 transition-all">
                    Посмотреть
                </button>
            </div>

        </article>
    );
}